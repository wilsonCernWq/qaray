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

#include "renderer.h"
#include <chrono>
#include <mutex>

namespace qaray {
///--------------------------------------------------------------------------//
float LinearToSRGB(const float c)
{
  const float a = 0.055f;
  if (c < 0.0031308f) { return 12.92f * c; }
  else { return (1.f + a) * POW(c, 1.f / 2.4f) - a; }
}
///--------------------------------------------------------------------------//
enum TimeState { START_FRAME, STOP_FRAME, KILL_FRAME };
void TimeFrame(TimeState state)
{
  static float avgRenderTime = 0.0f;
  static std::chrono::time_point<std::chrono::system_clock> startTime, endTime;
  static int numFrames = -1; // don't count the first frame
  if (state == START_FRAME) {
    startTime = std::chrono::system_clock::now();
  } else if (state == STOP_FRAME) {
    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    printf("\nElapsed Time is %f s\n", elapsed_seconds.count());
    if (++numFrames > 0) { // Update moving average
      avgRenderTime += (elapsed_seconds.count() - avgRenderTime) / numFrames;
    }
  } else {
    printf("\nProgram Ends, Average Frame Time %f s\n\n",
           avgRenderTime);
  }
}
void Renderer::StartTimer() { TimeFrame(START_FRAME); }
void Renderer::StopTimer() { TimeFrame(STOP_FRAME); }
void Renderer::KillTimer() { TimeFrame(KILL_FRAME); }
///--------------------------------------------------------------------------//
/// Constructor
///--------------------------------------------------------------------------//
Renderer::Renderer(RendererParam &param) : param(param)
{
  tasking::signal_start();
}
void Renderer::ComputeScene(FrameBuffer &fb, Scene &sc)
{
  //! scene
  image = &fb;
  scene = &sc;
  //! camera
  dof = scene->camera.depthOfField;
  focal = scene->camera.focalDistance;
  aspect = scene->camera.imgWidth / static_cast<float>(scene->camera.imgHeight);
  screenH = 2.f * focal * std::tan(scene->camera.fovy * PI / 2.f / 180.f);
  screenW = aspect * screenH;
  Point3 X = normalize(cross(scene->camera.dir, scene->camera.up));
  Point3 Y = normalize(cross(X, scene->camera.dir));
  Point3 Z = normalize(-scene->camera.dir);
  screenU = X * (screenW / scene->camera.imgWidth);
  screenV = -Y * (screenH / scene->camera.imgHeight);
  screenA =
      scene->camera.pos - Z * focal + Y * screenH / 2.f - X * screenW / 2.f;
  screenX = X;
  screenY = Y;
  screenZ = Z;
  //! canvas
  pixelW = static_cast<size_t>(scene->camera.imgWidth);
  pixelH = static_cast<size_t>(scene->camera.imgHeight);
  pixelRegion[0] = pixelRegion[1] = 0;
  pixelRegion[2] = pixelW;
  pixelRegion[3] = pixelH;
  pixelSize[0] = pixelRegion[2] - pixelRegion[0];
  pixelSize[1] = pixelRegion[3] - pixelRegion[1];
  //! frame
  image->Init(static_cast<int>(pixelSize[0]),
              static_cast<int>(pixelSize[1])); /* reinitialize local fb */
  colorBuffer = image->GetPixels();
  depthBuffer = image->GetZBuffer();
  sampleCountBuffer = image->GetSampleCount();
  irradianceCountBuffer = image->GetIrradianceComputationImage();
  maskBuffer = image->GetMasks();
  //! tiles
  tileDimX = static_cast<size_t>(CEIL(static_cast<float>(pixelSize[0]) /
      static_cast<float>(tileSize)));
  tileDimY = static_cast<size_t>(CEIL(static_cast<float>(pixelSize[1]) /
      static_cast<float>(tileSize)));
  tileCount = tileDimX * tileDimY;
  //-------------------------------------------------------------------------//
  // Initialize Photon Map
  //-------------------------------------------------------------------------//
  // TODO: Photon Map for MPI Mode
  scene->usePhotonMap = param.usePhotonMap;
  if (param.photonMapSize > 0 &&
      param.causticsMapSize > 0 &&
      param.usePhotonMap)
  {
    //-----------------------------------------------------------------------//
    scene->photonmap.size = param.photonMapSize;
    scene->photonmap.radius = param.photonMapRadius;
    scene->photonmap.bounce = param.photonMapBounce;
    scene->causticsmap.size = param.causticsMapSize;
    scene->causticsmap.radius = param.causticsMapRadius;
    scene->causticsmap.bounce = param.causticsMapBounce;
    std::chrono::time_point<std::chrono::system_clock> t1, t2;
    //! find out all point lights
    std::vector<Light *> photonLights;
    for (auto &light : scene->lights) {
      if (light->IsPhotonSource()) { photonLights.push_back(light); }
    }
    const qaFLOAT lightScale = 1.f / static_cast<qaFLOAT>(photonLights.size());
    //-----------------------------------------------------------------------//
    // Photon Map
    //-----------------------------------------------------------------------//
    {
      //---------------------------------------------------------------------//
      t1 = std::chrono::system_clock::now();
      //---------------------------------------------------------------------//
      scene->photonmap.map.CreateAllPhotons(param.photonMapSize);
      //! generate the photon map
      qaUINT numPhotonsRec(0);
      qaUINT numOfEmittedRays(0);
      while (true) {
        Light *light;
        //! randomly pick a light
        if (photonLights.size() == 1) { light = photonLights[0]; }
        else {
          qaFLOAT r;
          rng->local().Get1f(r);
          size_t id = MIN(static_cast<size_t>(CEIL(r * photonLights.size())),
                          photonLights.size() - 1);
          light = photonLights[id];
        }
        //! generate one photons
        DiffRay ray = light->RandomPhoton();
        ray.Normalize();
        DiffHitInfo hInfo;
        hInfo.Init();
        Color3f intensity = light->GetPhotonIntensity(ray.c.dir) * lightScale;
        qaBOOL finished = false; // whether the map is filled
        qaBOOL recorded = false; // whether a photon is recorded
        //! trace photon
        size_t bounce = 0;
        while (bounce < param.photonMapBounce) {
          //! trace the photon
          if (scene->TraceNodeNormal(scene->rootNode, ray, hInfo)) {
            const Material *mtl = hInfo.c.node->GetMaterial();
            //! if it is a diffuse surface
            if (mtl->IsPhotonSurface(0) && bounce != 0) {
              //! fetch a photon index
              size_t idx = numPhotonsRec++;
              //! check if the map is filled
              if (idx >= param.photonMapSize) {
                finished = true;
                break;
              }
              else {
                scene->photonmap.map[idx].position = hInfo.c.p;
                scene->photonmap.map[idx].SetDirection(ray.c.dir);
                scene->photonmap.map[idx].SetPower(intensity);
                recorded = true;
              }
            }
            if (mtl->RandomPhotonBounce(ray, intensity, hInfo)) {
              ++bounce;
              ray.Normalize();
              hInfo.Init();
            } else { break; }
          } else { break; }
        }
        if (recorded) { ++numOfEmittedRays; }
        if (finished) { break; }
      }
      scene->photonmap.map.ScalePhotonPowers(1.f / numOfEmittedRays);
      scene->photonmap.map.PrepareForIrradianceEstimation();
      //---------------------------------------------------------------------//
      t2 = std::chrono::system_clock::now();
      std::chrono::duration<double> dt = t2 - t1;
      printf("\nPhoton Map Takes %f s to Build\n", dt.count());
      FILE *fp = fopen("photonmap.dat", "wb");
      auto x = scene->photonmap.map.NumPhotons();
      fwrite(scene->photonmap.map.GetPhotons(), sizeof(cyPhotonMap::Photon),
             scene->photonmap.map.NumPhotons(), fp);
      fclose(fp);
      //---------------------------------------------------------------------//
    }
    //-----------------------------------------------------------------------//
    // Caustics Map
    //-----------------------------------------------------------------------//
    {
      //---------------------------------------------------------------------//
      t1 = std::chrono::system_clock::now();
      //---------------------------------------------------------------------//
      scene->causticsmap.map.CreateAllPhotons(param.causticsMapSize);
      //! generate the photon map
      qaUINT numPhotonsRec(0);
      qaUINT numOfEmittedRays(0);
      while (true) {
        Light *light;
        //! randomly pick a light
        if (photonLights.size() == 1) { light = photonLights[0]; }
        else {
          qaFLOAT r;
          rng->local().Get1f(r);
          size_t id = MIN(static_cast<size_t>(CEIL(r * photonLights.size())),
                          photonLights.size() - 1);
          light = photonLights[id];
        }
        //! generate one photons
        DiffRay ray = light->RandomPhoton();
        ray.Normalize();
        DiffHitInfo hInfo;
        hInfo.Init();
        Color3f intensity = light->GetPhotonIntensity(ray.c.dir) * lightScale;
        qaBOOL finished = false; // whether the map is filled
        qaBOOL recorded = false; // whether a photon is recorded
        //! trace photon
        size_t bounce = 0;
        while (bounce < param.causticsMapBounce) {
          //! trace the photon
          if (scene->TraceNodeNormal(scene->rootNode, ray, hInfo)) {
            const Material *mtl = hInfo.c.node->GetMaterial();
            //! if it is a diffuse surface
            if (mtl->IsPhotonSurface(0) &&
                !hInfo.c.hasDiffuseHit &&
                bounce != 0)
            {
              //! fetch a photon index
              size_t idx = numPhotonsRec++;
              //! check if the map is filled
              if (idx >= param.causticsMapSize) {
                finished = true;
                break;
              }
              else {
                scene->causticsmap.map[idx].position = hInfo.c.p;
                scene->causticsmap.map[idx].SetDirection(ray.c.dir);
                scene->causticsmap.map[idx].SetPower(intensity);
                recorded = true;
              }
            }
            if (mtl->RandomPhotonBounce(ray, intensity, hInfo)) {
              bool diffuseHit = hInfo.c.hasDiffuseHit;
              ++bounce;
              ray.Normalize();
              hInfo.Init();
              hInfo.c.hasDiffuseHit = (diffuseHit || mtl->IsPhotonSurface(0));
            } else { break; }
          } else { break; }
        }
        if (recorded) { ++numOfEmittedRays; }
        if (finished) { break; }
      }
      scene->causticsmap.map.ScalePhotonPowers(1.f / numOfEmittedRays);
      scene->causticsmap.map.PrepareForIrradianceEstimation();
      //---------------------------------------------------------------------//
      t2 = std::chrono::system_clock::now();
      std::chrono::duration<double> dt = t2 - t1;
      printf("\nCaustics Map Takes %f s to Build\n", dt.count());
      FILE *fp = fopen("caustics.dat", "wb");
      auto x = scene->causticsmap.map.NumPhotons();
      fwrite(scene->causticsmap.map.GetPhotons(), sizeof(cyPhotonMap::Photon),
             scene->causticsmap.map.NumPhotons(), fp);
      fclose(fp);
      //---------------------------------------------------------------------//
    }
  }
};
void Renderer::Init() {}
void Renderer::Terminate()
{
  scene->photonmap.Clear();
  scene->causticsmap.Clear();
};
///--------------------------------------------------------------------------//
/// Render each individual pixel
///--------------------------------------------------------------------------//
void Renderer::PixelRender(size_t i, size_t j, size_t tile_idx)
{
  // initializations
  SuperSamplerHalton sampler(Color3f(0.005f, 0.001f, 0.005f),
                             static_cast<int>(param.sppMin),
                             static_cast<int>(param.sppMax));
  float depth = 0.0f;
  // start looping
  while (sampler.Loop()) {
    // calculate one sample
    const Point3 texpos = sampler.NewPixelSample() + Point3(i, j, 0.f);
    const Point3 cpt = screenA + texpos.x * screenU + texpos.y * screenV;
    const Point3
        xpt = screenA + (texpos.x + DiffRay::dx) * screenU + texpos.y * screenV;
    const Point3
        ypt = screenA + texpos.x * screenU + (texpos.y + DiffRay::dy) * screenV;
    Point3 campos = scene->camera.pos;
    if (dof > 0.1f) {
      const Point3 dofSample = sampler.NewDofSample(dof);
      campos += dofSample.x * screenX + dofSample.y * screenY;
    }
    DiffRay ray(campos, cpt - campos,
                campos, xpt - campos,
                campos, ypt - campos);
    ray.Normalize();
    DiffHitInfo hInfo;
    hInfo.c.z = BIGFLOAT;
    bool hasHit = scene->TraceNodeNormal(scene->rootNode, ray, hInfo);
    Color3f localColor;
    if (hasHit) {
      localColor =
          hInfo.c.node->GetMaterial()->Shade(ray, hInfo, scene->lights,
                                             Material::maxBounce);
    } else {
      const float u = texpos.x / pixelW;
      const float v = texpos.y / pixelH;
      localColor = scene->background.Sample(Point3(u, v, 0.f));
    }
    // calculate depth for the first sample only
    if (sampler.GetSampleID() == 0) { depth = hasHit ? hInfo.c.z : BIGFLOAT; }
    // calculate moving average
    sampler.Accumulate(localColor);
    // increment
    sampler.Increment();
  }
  // post process color3f
  Color3f color = sampler.GetColor();
  if (param.useSRGB) {
    color.r = LinearToSRGB(color.r);
    color.g = LinearToSRGB(color.g);
    color.b = LinearToSRGB(color.b);
  }
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
  // write to frame buffer
  const size_t idx = (j - pixelRegion[1]) * pixelSize[0] + i - pixelRegion[0];
  colorBuffer[idx].r = static_cast<qaUCHAR>(roundf(color.r * 255.f));
  colorBuffer[idx].g = static_cast<qaUCHAR>(roundf(color.g * 255.f));
  colorBuffer[idx].b = static_cast<qaUCHAR>(roundf(color.b * 255.f));
  depthBuffer[idx] = depth;
  sampleCountBuffer[idx] = static_cast<qaUCHAR>(255.f * sampler.GetSampleID() /
      static_cast<qaFLOAT >(param.sppMax));
  maskBuffer[idx] = 1;
}
///--------------------------------------------------------------------------//
/// Setup rendering tasks for each threads
///--------------------------------------------------------------------------//
void Renderer::ThreadRender()
{
  //-------------------------------------------------------------------------//
  // Start timing
  //-------------------------------------------------------------------------//
  StartTimer();
  //-------------------------------------------------------------------------//
  // Rendering
  //-------------------------------------------------------------------------//
  if (mpiRank == 0) {
    printf("\nRunning with %zu threads on rank %zu\n",
           tasking::get_num_of_threads(), mpiRank);
  }
  const auto tileStart(static_cast<size_t>(mpiRank));
  const auto tileStop(static_cast<size_t>(tileCount));
  const auto tileStep(static_cast<size_t>(mpiSize));
  tasking::init();
  tasking::parallel_for(tileStart, tileStop, tileStep, [&](size_t k) {
    const size_t tileX(k % tileDimX);
    const size_t tileY(k / tileDimX);
    const size_t iStart =
        MIN(pixelRegion[2], tileX * tileSize + pixelRegion[0]);
    const size_t jStart =
        MIN(pixelRegion[3], tileY * tileSize + pixelRegion[1]);
    const size_t iEnd =
        MIN(pixelRegion[2], (tileX + 1) * tileSize + pixelRegion[0]);
    const size_t jEnd =
        MIN(pixelRegion[3], (tileY + 1) * tileSize + pixelRegion[1]);
    const size_t numPixels = (iEnd - iStart) * (jEnd - jStart);
    tasking::parallel_for(size_t(0), numPixels, size_t(1), [=](size_t idx) {
      const size_t j = jStart + idx / (iEnd - iStart);
      const size_t i = iStart + idx % (iEnd - iStart);
      if (!tasking::has_stop_signal()) { PixelRender(i, j, k); }
    });
    image->IncrementNumRenderPixel(static_cast<int>(numPixels));
  });
  //-------------------------------------------------------------------------//
  // Stop timing
  //-------------------------------------------------------------------------//
  StopTimer();
}
}
