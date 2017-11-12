#include "globalvar.h"
#include "xmlload.h"
#include "viewport.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>      /* C11 std::thread */
#include <atomic>      /* C11 */
#include <algorithm>

#ifdef USE_TBB
# include <tbb/task_arena.h>
# include <tbb/task_scheduler_init.h>
# include <tbb/parallel_for.h>
#endif

#ifdef USE_OMP
# include <omp.h>
#endif

#ifdef USE_MPI
# include <mpi.h>
#endif

//#undef USE_TBB
//#undef USE_OMP
//#undef USE_MPI
#pragma warning(disable: 588)
#define MULTITHREAD 1
//-------------------------------------------------------------------------
// Parameters
static const float PI = std::acos(-1.f);
static int bounce = 2;
static int sppMax = 64, sppMin = 4;
// Camera parameters
static float nearClip = 10.0f, dof = 0.0f;
static float  screenW, screenH, aspect;
static Point3 screenX, screenY, screenZ;
static Point3 screenU, screenV, screenA;
// Frame Buffer parameters
static Color24 *colorBuffer = NULL; // RGB uchar
static float   *depthBuffer = NULL;
static uchar   *sampleCountBuffer = NULL;
static uchar   *maskBuffer = NULL;
static int pixelW, pixelH;       // global size in pixel
static int pixelRegion[4] = {0}; // local image offset [x y]
static int pixelSize[2]   = {0}; // local image size
//-------------------------------------------------------------------------
// Parameters used for multi-threading
static const int tileSize = 32; // this value should be platform dependent
static int tileDimX = 0, tileDimY = 0;
static int threadSize = 1;
static std::atomic<bool> threadStop(false);
static std::thread *threadMain = nullptr;
//-------------------------------------------------------------------------
// Parameters used for MPI
static int mpiSize = 1;
static int mpiRank = 0;
static std::string mpiPrefix;

//-------------------------------------------------------------------------
//
void PixelRender(const int i, const int j)
{
  // initializations
  SuperSamplerHalton sampler(Color(0.005f, 0.001f, 0.005f), sppMin, sppMax);
  float depth = 0.0f;
  // start looping
  while (sampler.Loop())
  {
    // calculate one sample
    const Point3 texpos = sampler.NewPixelSample() + Point3(i, j, 0.f);
    const Point3 cpt = screenA + texpos.x * screenU + texpos.y * screenV;
    const Point3 xpt = screenA + (texpos.x + DiffRay::dx) * screenU + texpos.y * screenV;
    const Point3 ypt = screenA + texpos.x * screenU + (texpos.y + DiffRay::dy) * screenV;
    Point3 campos = camera.pos;
    if (dof > 0.1f) {
      const Point3 dofSample = sampler.NewDofSample(dof);
      campos += dofSample.x * screenX + dofSample.y * screenY;
    }
    DiffRay ray(campos, cpt - campos,
                campos, xpt - campos,
                campos, ypt - campos);
    ray.Normalize();
    DiffHitInfo hInfo;
    bool hasHit = TraceNodeNormal(rootNode, ray, hInfo);
    Color localColor;
    if (hasHit) {
      localColor = hInfo.c.node->GetMaterial()->Shade(ray, hInfo, lights, bounce);
    } else {
      const float u = texpos.x / pixelW;
      const float v = texpos.y / pixelH;
      localColor = background.Sample(Point3(u, v, 0.f));
    }
    // calculate depth for the first sample only
    if (sampler.GetSampleID() == 0) { depth = hasHit ? hInfo.c.z : BIGFLOAT; }
    // calculate moving average
    sampler.Accumulate(localColor);
    // increment
    sampler.Increment();
  }
  // write to framebuffer
  const int idx = (j - pixelRegion[1]) * pixelSize[0] + i - pixelRegion[0];
  colorBuffer[idx].r = static_cast<uchar>(round(sampler.GetColor().r * 255.f));
  colorBuffer[idx].g = static_cast<uchar>(round(sampler.GetColor().g * 255.f));
  colorBuffer[idx].b = static_cast<uchar>(round(sampler.GetColor().b * 255.f));
  depthBuffer[idx] = depth;
  sampleCountBuffer[idx] = 255.f * sampler.GetSampleID() / (float)sppMax;
  maskBuffer[idx] = 1;
}

void ThreadRender()
{
  // Start timing
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  double t1, t2;
  if (mpiSize == 1) { TimeFrame(START_FRAME); }
  else {
    t1 = MPI_Wtime();
  }
#else
  TimeFrame(START_FRAME);
#endif
  // Rendering
  if (mpiRank == 0) {
    printf("\nRunning with %d threads on rank %d\n", threadSize, mpiRank);
  }
  const auto tileSta(static_cast<size_t>(mpiRank));
  const auto tileNum(static_cast<size_t>(tileDimX * tileDimY));
  const auto tileStp(static_cast<size_t>(mpiSize));
#if defined(USE_TBB) && MULTITHREAD
  tbb::task_scheduler_init init(threadSize); // Explicit number of threads
  tbb::parallel_for(tileSta, tileNum, tileStp, [=] (size_t k) {
#else
# if MULTITHREAD
  omp_set_num_threads(threadSize);
# endif
  for (size_t k = tileSta; k < tileNum; k += tileStp) {
#endif
    const int tileX = static_cast<int>(k % tileDimX);
    const int tileY = static_cast<int>(k / tileDimX);
    const int iStart =
        MIN(pixelRegion[2], tileX * tileSize + pixelRegion[0]);
    const size_t jStart =
        MIN(pixelRegion[3], tileY * tileSize + pixelRegion[1]);
    const size_t iEnd =
        MIN(pixelRegion[2], (tileX + 1) * tileSize + pixelRegion[0]);
    const size_t jEnd =
        MIN(pixelRegion[3], (tileY + 1) * tileSize + pixelRegion[1]);
    const size_t numPixels = (iEnd - iStart) * (jEnd - jStart);
#if defined(USE_TBB) && MULTITHREAD
    tbb::parallel_for(size_t(0), numPixels, [=] (size_t idx) {
#else
# if MULTITHREAD
#   pragma omp parallel for 
# endif
    for (size_t idx(0); idx < numPixels; ++idx) {
#endif
      const size_t j = jStart + idx / (iEnd - iStart);
      const size_t i = iStart + idx % (iEnd - iStart);
      if (!threadStop) { PixelRender(i, j); }
#if defined(USE_TBB) && MULTITHREAD
      });
#else
    }
#endif
    renderImage.IncrementNumRenderPixel(numPixels); // thread safe
#if defined(USE_TBB) && MULTITHREAD
    });
#else
  };
#endif
  // End timing
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpiSize == 1) { TimeFrame(STOP_FRAME); }
  else {
    t2 = MPI_Wtime();
    if (mpiRank == 0) printf("\nElapsed time is %f\n", t2 - t1);
  }
#else
  TimeFrame(STOP_FRAME);
#endif
}

//-------------------------------------------------------------------------
// Initialize the scene parameters
void DivideLength
    (const size_t len, const size_t div, const size_t idx, size_t &left, size_t &right)
{
  const size_t H = MAX(1, len / div);
  if (idx != div - 1) {
    left = MIN((idx) * H, len);
    right = MIN((idx + 1) * H, len);
  } else {
    left = MIN((idx) * H, len);
    right = len;
  }
}

void ComputeScene() {
  // rendering
  nearClip = camera.focaldist;
  dof = camera.dof;
  pixelW = camera.imgWidth;
  pixelH = camera.imgHeight;
  aspect =
      static_cast<float>(camera.imgWidth) /
      static_cast<float>(camera.imgHeight);
  screenH = 2.f * nearClip * std::tan(camera.fov * PI / 2.f / 180.f);
  screenW = aspect * screenH;
  Point3 X = glm::normalize(glm::cross(camera.dir,camera.up));
  Point3 Y = glm::normalize(glm::cross(X,camera.dir));
  Point3 Z = glm::normalize(-camera.dir);
  screenU = X * (screenW / camera.imgWidth);
  screenV = -Y * (screenH / camera.imgHeight);
  screenA = camera.pos
            - Z * nearClip
            + Y * screenH / 2.f
            - X * screenW / 2.f;
  screenX = X; screenY = Y; screenZ = Z;
  // MPI range
  pixelRegion[0] = pixelRegion[1] = 0;
  pixelRegion[2] = pixelW;
  pixelRegion[3] = pixelH;
  pixelSize[0] = pixelRegion[2] - pixelRegion[0];
  pixelSize[1] = pixelRegion[3] - pixelRegion[1];
  renderImage.Init(pixelSize[0], pixelSize[1]); /* re-initialize local image */
  colorBuffer = renderImage.GetPixels();
  depthBuffer = renderImage.GetZBuffer();
  sampleCountBuffer = renderImage.GetSampleCount();
  maskBuffer = renderImage.GetMasks();
  // multi-threading
  tileDimX = CEIL(static_cast<float>(pixelSize[0]) /
                  static_cast<float>(tileSize));
  tileDimY = CEIL(static_cast<float>(pixelSize[1]) /
                  static_cast<float>(tileSize));
}

//------------------------------------------------------------------------
// Called to start rendering (renderer must run in a separate thread)
void BeginRender()
{
  // Reset
  StopRender();
  renderImage.ResetNumRenderedPixels();
  // Start threads
  threadStop = false;
  threadMain = new std::thread(ThreadRender);
}

// Called to end rendering (if it is not already finished)
void StopRender()
{
  // Send stop signal
  threadStop = true;
  // Wait for threads to finish
  if (threadMain != nullptr) {
    threadMain->join();
    delete threadMain;
    threadMain = nullptr;
  }
}

// Called when the rendering is end successfully
void CleanRender()
{
  // Save image
  renderImage.ComputeZBufferImage();
  renderImage.ComputeSampleCountImage();
  renderImage.SaveImage("colorBuffer.png");
  renderImage.SaveZImage("depthBuffer.png");
  renderImage.SaveSampleCountImage("sampleBuffer.png");
}

// Called when the program is stopped
void KillRender()
{
  StopRender();
  TimeFrame(KILL_FRAME);
}

void OnlineRender()
{
#ifdef USE_GUI
  if (mpiSize == 1) { ShowViewport(); }
  else {
    std::cerr << "Warning: Trying to use GUI window in mpi mode" << std::endl;
  }
#else
  std::cerr <<
    "Warning: GUI Mode is not enabled. "
    "Add '-DENABLE_GUI' to the cmake and recompile the program" 
      << std::endl;
#endif
}

//-------------------------------------------------------------------------
//
template<typename T>
void PlaceImage
    (int srcext[4], size_t dstW, size_t dstH, const uchar *mask, const T *src, T *dst)
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

void BatchRender()
{
  //-- render locally
  renderImage.ResetNumRenderedPixels();
  threadStop = false;
  ThreadRender();
  threadStop = true;
  //renderImage.ComputeZBufferImage();
  //renderImage.ComputeSampleCountImage();
  //renderImage.SaveZImage((mpiPrefix+"depthBuffer.png").c_str()); 
  //renderImage.SaveImage ((mpiPrefix+"colorBuffer.png").c_str());
  //renderImage.SaveSampleCountImage((mpiPrefix+"sampleBuffer.png").c_str());
#ifdef USE_MPI
  //-- gather data in rank 0
  MPI_Barrier(MPI_COMM_WORLD);
  size_t master = 0;
  int tag[5] = {100, 200, 300, 400, 500};
  if (mpiRank == master) { // reveive data
    RenderImage finalImage;
    finalImage.Init(pixelW, pixelH);
    for (int target = 0; target < mpiSize; ++target) {
      if (target == master) {
        int imgext[4] = {(int) pixelRegion[0], (int) pixelRegion[1],
                         (int) pixelRegion[2], (int) pixelRegion[3]};
        PlaceImage<Color24>(imgext, pixelW, pixelH, maskBuffer, colorBuffer,
                            finalImage.GetPixels());
        PlaceImage<float>  (imgext, pixelW, pixelH, maskBuffer, depthBuffer,
			    finalImage.GetZBuffer());
	PlaceImage<uchar>  (imgext, pixelW, pixelH, maskBuffer, sampleCountBuffer,
			    finalImage.GetSampleCount());	
      } else {
        int imgext[4];
        MPI_Recv(&imgext, 4, MPI_INT, target, tag[0],
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int buffsize = (imgext[2] - imgext[0]) * (imgext[3] - imgext[1]);
        Color24 *cbuff = new Color24[buffsize];
        float *zbuff = new float[buffsize];
        uchar *sbuff = new uchar[buffsize];
        uchar *mbuff = new uchar[buffsize];
        MPI_Recv(cbuff, buffsize * sizeof(Color24), MPI_BYTE,
                 target, tag[1], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(zbuff, buffsize, MPI_FLOAT, target,
                 tag[2], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(sbuff, buffsize * sizeof(uchar), MPI_BYTE, target,
                 tag[3], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(mbuff, buffsize * sizeof(uchar), MPI_BYTE, target,
                 tag[4], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        PlaceImage<Color24>(imgext, pixelW, pixelH, mbuff, cbuff,
                            finalImage.GetPixels());
        PlaceImage<float>  (imgext, pixelW, pixelH, mbuff, zbuff,
			    finalImage.GetZBuffer());
        PlaceImage<uchar>  (imgext, pixelW, pixelH, mbuff, sbuff,
			    finalImage.GetSampleCount());
      }
    }
    finalImage.IncrementNumRenderPixel(pixelW * pixelH);
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
    MPI_Send(colorBuffer, buffsize * sizeof(Color24), MPI_BYTE,
             master, tag[1], MPI_COMM_WORLD);
    MPI_Send(depthBuffer, buffsize, MPI_FLOAT,
             master, tag[2], MPI_COMM_WORLD);
    MPI_Send(sampleCountBuffer, buffsize * sizeof(uchar), MPI_BYTE,
             master, tag[3], MPI_COMM_WORLD);
    MPI_Send(maskBuffer, buffsize * sizeof(uchar), MPI_BYTE,
             master, tag[4], MPI_COMM_WORLD);
  }
#else
  renderImage.ComputeZBufferImage();
  renderImage.ComputeSampleCountImage();
  renderImage.SaveImage ("colorBuffer_LOCAL.png");
  renderImage.SaveZImage("depthBuffer_LOCAL.png");
  renderImage.SaveSampleCountImage("sampleBuffer_LOCAL.png");
#endif
}

//-------------------------------------------------------------------------
int main(int argc, char **argv)
{
#ifdef USE_MPI
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  // Setup parameters
  if (mpiRank == 0) { fprintf(stdout, "Number of MPI Ranks: %i\n", mpiSize); }
  mpiPrefix = std::string("rank_") + std::to_string(mpiRank) + "_";
#endif

#if defined(USE_TBB)
# if TBB_INTERFACE_VERSION >= 9100
  threadSize = tbb::this_task_arena::max_concurrency();
# else
  threadSize = tbb::task_scheduler_init::default_num_threads();
# endif
#elif defined(USE_OMP)
# pragma omp parallel
  {
  threadSize = omp_get_num_threads();
  }
#endif
  bool batchmode = false;

  // Parse CMD arguments
  const char *xmlfile;
  if (argc < 2) {
    std::cerr << "Error: insufficient input" << std::endl;
    return -1;
  }
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str.compare("-batch") == 0) {
      batchmode = true;
    } else if (str.compare("-spp") == 0) {
      sppMin = std::atoi(argv[++i]);
      sppMax = sppMin;
    } else if (str.compare("-sppMin") == 0) {
      sppMin = std::atoi(argv[++i]);
    } else if (str.compare("-sppMax") == 0) {
      sppMax = std::atoi(argv[++i]);
    } else if (str.compare("-bounce") == 0) {
      bounce = std::atoi(argv[++i]);
    } else if (str.compare("-threads") == 0) {
      int tmp = std::atoi(argv[++i]);
      if (0 < tmp && tmp < threadSize) { threadSize = tmp; }
    } else {
      xmlfile = argv[i];
    }
  }

  // Parse XML input file
  if (mpiRank != 0) { LoadSceneInSilentMode(true); }
  LoadScene(xmlfile);
  ComputeScene();
  if (batchmode) {
    BatchRender();
  } else {
    OnlineRender();
  }

#ifdef USE_MPI
  // Finalize the MPI environment.
  MPI_Finalize();
#endif

  return 0;
}
