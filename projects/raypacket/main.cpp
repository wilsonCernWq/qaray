#include "globalvar.h"
#include "xmlload.h"
#include "viewport.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>      /* C11 std::thread */
#include <atomic>      /* C11 */
#include <algorithm>

#ifdef USE_GUI /* MPI will not work with GUI mode */
# undef USE_MPI
#endif

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
// Parameters used for rendering
static const float PI = std::acos(-1);
static const float nearClip = 10.0f;
static size_t spp = 1, bounce = 5;
static float  screenW, screenH, aspect;
static Point3 screenU, screenV, screenA;
static Color24* colorBuffer = NULL; // RGB uchar
static float*   depthBuffer = NULL;
static size_t pixelW, pixelH;       // global size in pixel
static size_t pixelRegion[4] = {0}; // local image offset [x y]
static size_t pixelSize[2]   = {0}; // local image size
//-------------------------------------------------------------------------
// Parameters used for multi-threading
static const size_t tileSize = 4; // this value should be platform dependent
static size_t tileDimX = 0, tileDimY = 0;
static std::atomic<bool> threadStop(false);
static std::thread*      threadMain = nullptr;
static int threadSize = 1;
//-------------------------------------------------------------------------
// Parameters used for MPI
static int mpiSize = 1;
static int mpiRank = 0;
static std::string mpiPrefix;
//-------------------------------------------------------------------------
//
void PixelRender(const size_t i, const size_t j)
{
  const size_t sppLen = CEIL(SQRT(spp));
  const float  sppStp = 1.f / static_cast<float>(sppLen);
  Color color(0.0f, 0.0f, 0.0f);
  float depth = 0.0f;
  for (size_t s = 0; s < spp; ++s) {
    const Point3 cpt =
      screenA +
      (i + (s % sppLen + 0.5f) * sppStp) * screenU +
      (j + (s / sppLen + 0.5f) * sppStp) * screenV;
    Ray ray(camera.pos, cpt - camera.pos); ray.Normalize();
    HitInfo hInfo;
    bool hasHit = TraceNodeNormal(rootNode, ray, hInfo);
    if (hasHit) {
      auto localColor = 
	hInfo.node->GetMaterial()->Shade(ray, hInfo, lights, bounce);
      color += (localColor - color) / static_cast<float>(s + 1);
      depth += (hInfo.z - depth) / static_cast<float>(s + 1);
    }
    else {
      color += (-color) / static_cast<float>(s + 1);
      depth += (BIGFLOAT - depth) / static_cast<float>(s + 1);
    }
  }
  const size_t idx = (j - pixelRegion[1]) * pixelSize[0] + i - pixelRegion[0];
  colorBuffer[idx].r = static_cast<uchar>(round(color.r * 255.f));
  colorBuffer[idx].g = static_cast<uchar>(round(color.g * 255.f));
  colorBuffer[idx].b = static_cast<uchar>(round(color.b * 255.f));
  depthBuffer[idx] = depth;
}
void ThreadRender()
{
  // Start timing
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  double t1 = MPI_Wtime(); 
#else
  TimeFrame(START_FRAME);
#endif

  // Rendering
  if (mpiRank == 0) { 
  printf("\nRunning with %d threads on rank %d\n", threadSize, mpiRank); 
  }
#if defined(USE_TBB) && MULTITHREAD
  tbb::task_scheduler_init init(threadSize); // Explicit number of threads 
  tbb::parallel_for(size_t(0), size_t(tileDimX*tileDimY), [=] (size_t k) {
#else
# if MULTITHREAD
  omp_set_num_threads(threadSize);
#  pragma omp parallel for 
# endif
  for (size_t k = size_t(0); k < size_t(tileDimX*tileDimY); ++k) {
#endif
    const size_t tileX = k % tileDimX;
    const size_t tileY = k / tileDimX;
    const size_t iStart = 
      MIN(pixelRegion[2], tileX * tileSize + pixelRegion[0]);
    const size_t jStart = 
      MIN(pixelRegion[3], tileY * tileSize + pixelRegion[1]);
    const size_t iEnd = 
      MIN(pixelRegion[2], (tileX + 1) * tileSize + pixelRegion[0]);
    const size_t jEnd = 
      MIN(pixelRegion[3], (tileY + 1) * tileSize + pixelRegion[1]);
    for (size_t j = jStart; j < jEnd; ++j) {
      for (size_t i = iStart; i < iEnd; ++i) {
        if (!threadStop) { PixelRender(i, j); }
      }
    }
    const size_t numRenderedPixel = (iEnd - iStart) * (jEnd - jStart);
    renderImage.IncrementNumRenderPixel(numRenderedPixel); // thread safe
#if defined(USE_TBB) && MULTITHREAD
  });
#else
  }
#endif

  // End timing
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  double t2 = MPI_Wtime(); 
  if (mpiRank == 0) printf("\nElapsed time is %f\n", t2 - t1); 
#else
  TimeFrame(STOP_FRAME);
#endif
}
//-------------------------------------------------------------------------
// Initialize the scene parameters
void DivideLength
(const size_t len, const size_t div, const size_t idx, 
 size_t& left, size_t& right)
{
  const size_t H = MAX(1, len / div);
  if (idx != div - 1) {
    left  = MIN((idx    ) * H, len);
    right = MIN((idx + 1) * H, len);
  } else {
    left  = MIN((idx    ) * H, len);
    right = len;
  }
}
void ComputeScene()
{
  // rendering
  pixelW = camera.imgWidth;
  pixelH = camera.imgHeight;
  aspect =
    static_cast<float>(camera.imgWidth) /
    static_cast<float>(camera.imgHeight);
  screenH = 2.f * nearClip * std::tan(camera.fov * PI / 2.f / 180.f);
  screenW = aspect * screenH;
  Point3 X = glm::normalize(glm::cross(camera.dir, camera.up));
  Point3 Y = glm::normalize(glm::cross(X, camera.dir));
  Point3 Z = glm::normalize(-camera.dir);
  screenU = X * (screenW / camera.imgWidth);
  screenV =-Y * (screenH / camera.imgHeight);
  screenA = camera.pos 
    - Z * nearClip 
    + Y * screenH / 2.f 
    - X * screenW / 2.f;
  // MPI setting
#ifdef USE_MPI
  const size_t mpiSizeX = mpiSize;
  const size_t mpiSizeY = 1;
  const size_t mpiIdxX = mpiRank % mpiSizeX;
  const size_t mpiIdxY = mpiRank / mpiSizeX;
  DivideLength(pixelW, mpiSizeX, mpiIdxX, pixelRegion[0], pixelRegion[2]);
  DivideLength(pixelH, mpiSizeY, mpiIdxY, pixelRegion[1], pixelRegion[3]);
#else
  pixelRegion[0] = pixelRegion[1] = 0;
  pixelRegion[2] = pixelW;
  pixelRegion[3] = pixelH;
#endif
  pixelSize[0] = pixelRegion[2] - pixelRegion[0];
  pixelSize[1] = pixelRegion[3] - pixelRegion[1];
  renderImage.Init(pixelSize[0], pixelSize[1]); /* re-initialize local image */
  colorBuffer = renderImage.GetPixels();
  depthBuffer = renderImage.GetZBuffer();
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
  renderImage.SaveImage ("colorBuffer.png");
  renderImage.SaveZImage("depthBuffer.png");
}
// Called when the program is stopped
void KillRender()
{
  StopRender();
  TimeFrame(KILL_FRAME);
}
void OnlineRender() { 
#ifdef USE_GUI
  ShowViewport();
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
void PlaceImage(int srcext[4], size_t dstW, size_t dstH, const T* src, T* dst)
{
  const size_t xstart = CLAMP(srcext[0], 0, dstW);
  const size_t ystart = CLAMP(srcext[1], 0, dstH);
  const size_t xend   = CLAMP(srcext[2], 0, dstW);
  const size_t yend   = CLAMP(srcext[3], 0, dstH);
  const size_t srcW = srcext[2] - srcext[0];
  const size_t srcH = srcext[3] - srcext[1];
#pragma omp parallel for collapse(2)
  for (size_t j = ystart; j < yend; ++j) {
    for (size_t i = xstart; i < xend; ++i) {
      const size_t srcidx = (j - ystart) * srcW + i - xstart;
      const size_t dstidx = j * dstW + i;
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
  // renderImage.ComputeZBufferImage();
  // renderImage.SaveImage ((mpiPrefix+"colorBuffer.png").c_str());
  // renderImage.SaveZImage((mpiPrefix+"depthBuffer.png").c_str());
#ifdef USE_MPI
  //-- gather data in rank 0
  MPI_Barrier(MPI_COMM_WORLD);
  size_t master = 0;
  int tag[3] = {100, 200, 300};
  if (mpiRank == master) { // reveive data
    RenderImage finalImage; finalImage.Init(pixelW, pixelH);
    for (int target = 0; target < mpiSize; ++target) {
      if (target == master) {
	int imgext[4] = {(int)pixelRegion[0], (int)pixelRegion[1], 
			 (int)pixelRegion[2], (int)pixelRegion[3]};
	PlaceImage<Color24>(imgext, pixelW, pixelH, colorBuffer, 
			    finalImage.GetPixels());
	PlaceImage<float>  (imgext, pixelW, pixelH, depthBuffer, 
			    finalImage.GetZBuffer());
      }
      else {
	int imgext[4];
	MPI_Recv(&imgext, 4, MPI_INT, target, tag[0], 
		 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	int buffsize = (imgext[2] - imgext[0]) * (imgext[3] - imgext[1]);
	Color24* cbuff = new Color24[buffsize];
	float*   zbuff = new float  [buffsize];
	MPI_Recv(cbuff, buffsize * sizeof(Color24), MPI_BYTE, 
		 target, tag[1], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// printf("Receive ColorBuffer from rank %d \n", target);
	MPI_Recv(zbuff, buffsize, MPI_FLOAT, target, 
		 tag[2], MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// printf("Receive ZBuffer from rank %d \n", target);
	// debug(imgext[0]);
	// debug(imgext[1]);
	// debug(imgext[2]);
	// debug(imgext[3]);
	PlaceImage<Color24>(imgext, pixelW, pixelH, cbuff, 
			    finalImage.GetPixels());
	PlaceImage<float>  (imgext, pixelW, pixelH, zbuff, 
			    finalImage.GetZBuffer());
      }
    }
    finalImage.IncrementNumRenderPixel(pixelW*pixelH);
    finalImage.ComputeZBufferImage();
    finalImage.SaveImage ("colorBuffer_MPI.png");
    finalImage.SaveZImage("depthBuffer_MPI.png");
  }
  else {
    int imgext[4] = {(int)pixelRegion[0], (int)pixelRegion[1], 
		     (int)pixelRegion[2], (int)pixelRegion[3]};
    MPI_Send(&imgext, 4, MPI_INT, master, tag[0], MPI_COMM_WORLD);
    int buffsize = (imgext[2] - imgext[0]) * (imgext[3] - imgext[1]);    
    MPI_Send(colorBuffer, buffsize * sizeof(Color24), MPI_BYTE, 
             master, tag[1], MPI_COMM_WORLD);
    MPI_Send(depthBuffer, buffsize, MPI_FLOAT, 
	     master, tag[2], MPI_COMM_WORLD);
  }
#else
  renderImage.ComputeZBufferImage();
  renderImage.SaveImage ("colorBuffer_LOCAL.png");
  renderImage.SaveZImage("depthBuffer_LOCAL.png");
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
  char processor_name[MPI_MAX_PROCESSOR_NAME]; int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  // Print off a hello world message
  // printf("\nStarting processor %s, rank %d out of %d processors\n",
  //   processor_name, mpiRank, mpiSize);
  // Setup parameters
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
  const char* xmlfile;
  if (argc < 2) {
    std::cerr << "Error: insufficient input" << std::endl;
    return -1;
  }
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str.compare("-batch")==0) {
      batchmode = true;
    }
    else if (str.compare("-spp")==0) {
      spp = std::atoi(argv[++i]);
    }
    else if (str.compare("-bounce")==0) {
      bounce = std::atoi(argv[++i]);
    }
    else if (str.compare("-threads")==0) {
      int tmp = std::atoi(argv[++i]);
      if (0 < tmp && tmp < threadSize) { threadSize = tmp; }
    }
    else {
      xmlfile = argv[1];
    }
  }
 
  // Parse XML input file
  if (mpiRank != 0) 
  { 
    LoadSceneInSilentMode(true); 
  }
  LoadScene(xmlfile);
  ComputeScene();
  if (batchmode) {
    BatchRender();
  }
  else {
    OnlineRender();
  }

#ifdef USE_MPI
  // Finalize the MPI environment.
  MPI_Finalize();
#endif

  return 0;
}
