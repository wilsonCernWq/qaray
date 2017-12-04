///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/3/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.              //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//

#include "Renderer_MPI.h"
#include "parser/xmlload.h"

namespace qaray {
Renderer_MPI::Renderer_MPI(RendererParam& param) : Renderer(param) {}
void Renderer_MPI::Init()
{
#ifdef USE_MPI
  // Initialize the MPI environment
  MPI_Init(nullptr, nullptr);
  // Get the number of processes
  int tmp_mpi_size;
  MPI_Comm_size(MPI_COMM_WORLD, &tmp_mpi_size);
  mpiSize = size_t(tmp_mpi_size);
  // Get the rank of the process
  int tmp_mpi_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &tmp_mpi_rank);
  mpiRank = size_t(tmp_mpi_rank);
  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  // Setup parameters
  if (mpiRank == 0) { fprintf(stdout, "Number of MPI Ranks: %zu\n", mpiSize); }
  mpiPrefix = std::string("rank_") + std::to_string(mpiRank) + "_";
#endif
  if (mpiRank != 0) { LoadSceneInSilentMode(true); }
}
void Renderer_MPI::Terminate()
{
#ifdef USE_MPI
  MPI_Finalize(); // Finalize the MPI environment.
#endif
}
//---------------------------------------------------------------------------//
// Start timing
//---------------------------------------------------------------------------//
void Renderer_MPI::StartTimer()
{
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpiSize == 1) { TimeFrame(START_FRAME); }
  else {
    t1 = MPI_Wtime();
  }
#else
  Renderer::StartTimer();
#endif
}
//---------------------------------------------------------------------------//
// Stop timing
//---------------------------------------------------------------------------//
void Renderer_MPI::StopTimer()
{
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpiSize == 1) { TimeFrame(STOP_FRAME); }
  else {
    t2 = MPI_Wtime();
    if (mpiRank == 0) printf("\nElapsed time is %f\n", t2 - t1);
  }
#else
  Renderer::StartTimer();
#endif
}
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
template<typename T>
void PlaceImage(const int srcext[4], size_t dstW, size_t dstH,
                const qaUCHAR *mask, const T *src, T *dst)
{
  const size_t xstart = CLAMP(srcext[0], 0, dstW);
  const size_t ystart = CLAMP(srcext[1], 0, dstH);
  const size_t xend = CLAMP(srcext[2], 0, dstW);
  const size_t yend = CLAMP(srcext[3], 0, dstH);
  const size_t srcW = srcext[2] - srcext[0];
  const size_t srcH = srcext[3] - srcext[1];
# pragma omp parallel for collapse(2)
  for (size_t j = ystart; j < yend; ++j) {
    for (size_t i = xstart; i < xend; ++i) {
      const size_t srcidx = (j - ystart) * srcW + i - xstart;
      const size_t dstidx = j * dstW + i;
      if (mask[srcidx] != 0)
        dst[dstidx] = src[srcidx];
    }
  }
}
void Renderer_MPI::Render() {
  //-------------------------------------------------------------------------//
  // Render
  //-------------------------------------------------------------------------//
  // first we render locally
  renderImage->ResetNumRenderedPixels();
  threadStop = false;
  ThreadRender();
  threadStop = true;
  //-------------------------------------------------------------------------//
  // debug
  //renderImage->ComputeZBufferImage();
  //renderImage->ComputeSampleCountImage();
  //renderImage->SaveZImage((mpiPrefix+"depthBuffer.png").c_str());
  //renderImage->SaveImage ((mpiPrefix+"colorBuffer.png").c_str());
  //renderImage->SaveSampleCountImage((mpiPrefix+"sampleBuffer.png").c_str());
  //-------------------------------------------------------------------------//
  // now we gather images
  //-------------------------------------------------------------------------//
#ifdef USE_MPI
  //-- gather data in rank 0
  MPI_Barrier(MPI_COMM_WORLD);
  int master = 0;
  int tag[5] = {100, 200, 300, 400, 500};
  if (mpiRank == master) { // receive data
    RenderImage finalImage;
    finalImage.Init(static_cast<int>(pixelW), static_cast<int>(pixelH));
    for (int target = 0; target < mpiSize; ++target) {
      if (target == master) {
        int imgext[4] = {(int) pixelRegion[0], (int) pixelRegion[1],
                         (int) pixelRegion[2], (int) pixelRegion[3]};
        PlaceImage<Color3c>(imgext, pixelW, pixelH, maskBuffer,
                            colorBuffer,
                            finalImage.GetPixels());
        PlaceImage<float  >(imgext, pixelW, pixelH, maskBuffer,
                            depthBuffer,
                            finalImage.GetZBuffer());
        PlaceImage<qaUCHAR>(imgext, pixelW, pixelH, maskBuffer,
                            sampleCountBuffer,
                            finalImage.GetSampleCount());
      } else {
        int imgext[4];
        MPI_Recv(&imgext, 4, MPI_INT, target, tag[0],
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int buffsize = (imgext[2] - imgext[0]) * (imgext[3] - imgext[1]);
        auto cbuff = new Color3c[buffsize];
        auto zbuff = new float[buffsize];
        auto sbuff = new qaUCHAR[buffsize];
        auto mbuff = new qaUCHAR[buffsize];
        MPI_Recv(cbuff, buffsize * sizeof(Color3c), MPI_BYTE,
                 target, tag[1], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(zbuff, buffsize, MPI_FLOAT, target,
                 tag[2], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(sbuff, buffsize * sizeof(qaUCHAR), MPI_BYTE, target,
                 tag[3], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(mbuff, buffsize * sizeof(qaUCHAR), MPI_BYTE, target,
                 tag[4], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        PlaceImage<Color3c>(imgext, pixelW, pixelH, mbuff, cbuff,
                            finalImage.GetPixels());
        PlaceImage<float>(imgext, pixelW, pixelH, mbuff, zbuff,
                          finalImage.GetZBuffer());
        PlaceImage<qaUCHAR>(imgext, pixelW, pixelH, mbuff, sbuff,
                            finalImage.GetSampleCount());
      }
    }
    finalImage.IncrementNumRenderPixel(static_cast<int>(pixelW * pixelH));
    finalImage.ComputeZBufferImage();
    finalImage.ComputeSampleCountImage();
    finalImage.SaveImage("colorBuffer_MPI.png");
    finalImage.SaveZImage("depthBuffer_MPI.png");
    finalImage.SaveSampleCountImage("sampleBuffer_MPI.png");
  } else {
    int imgext[4] = {(int) pixelRegion[0], (int) pixelRegion[1],
                     (int) pixelRegion[2], (int) pixelRegion[3]};
    MPI_Send(&imgext, 4, MPI_INT, master, tag[0], MPI_COMM_WORLD);
    int buffsize = (imgext[2] - imgext[0]) * (imgext[3] - imgext[1]);
    MPI_Send(colorBuffer, buffsize * sizeof(Color3c), MPI_BYTE,
             master, tag[1], MPI_COMM_WORLD);
    MPI_Send(depthBuffer, buffsize, MPI_FLOAT,
             master, tag[2], MPI_COMM_WORLD);
    MPI_Send(sampleCountBuffer, buffsize * sizeof(qaUCHAR), MPI_BYTE,
             master, tag[3], MPI_COMM_WORLD);
    MPI_Send(maskBuffer, buffsize * sizeof(qaUCHAR), MPI_BYTE,
             master, tag[4], MPI_COMM_WORLD);
  }
#else
  renderImage->ComputeZBufferImage();
  renderImage->ComputeSampleCountImage();
  renderImage->SaveImage ("colorBuffer_LOCAL.png");
  renderImage->SaveZImage("depthBuffer_LOCAL.png");
  renderImage->SaveSampleCountImage("sampleBuffer_LOCAL.png");
#endif
}
}

