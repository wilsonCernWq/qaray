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

namespace qaray {
///--------------------------------------------------------------------------//
float LinearToSRGB(const float c)
{
  const float a = 0.055f;
  if (c < 0.0031308f) { return 12.92f * c; }
  else { return (1.f + a) * POW(c, 1.f / 2.4f) - a; }
}
///--------------------------------------------------------------------------//
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
    printf("\nRender time is %f s\n", elapsed_seconds.count());
    if (++numFrames > 0) { // Update moving average
      avgRenderTime += (elapsed_seconds.count() - avgRenderTime) / numFrames;
    }
  } else {
    printf("\nEnding the program, average frame time is %f s\n\n",
           avgRenderTime);
  }
}
///--------------------------------------------------------------------------//
/// Constructor
///--------------------------------------------------------------------------//
Renderer::Renderer(RendererParam& param) :
    param(param), threadStop(false)
{}
void Renderer::ComputeScene(RenderImage &fb, Scene &sc)
{
  //!
  renderImage = &fb;
  scene = &sc;
  //!
  focal = scene->camera.focalDistance;
  dof = scene->camera.depthOfField;
  aspect = static_cast<float>(scene->camera.imgWidth) /
      static_cast<float>(scene->camera.imgHeight);
  //!
  screenH =
      2.f * focal * std::tan(scene->camera.fovy * PI / 2.f / 180.f);
  screenW = aspect * screenH;
  Point3 X = normalize(cross(scene->camera.dir, scene->camera.up));
  Point3 Y = normalize(cross(X, scene->camera.dir));
  Point3 Z = normalize(-scene->camera.dir);
  screenU = X * (screenW / scene->camera.imgWidth);
  screenV = -Y * (screenH / scene->camera.imgHeight);
  screenA = scene->camera.pos
      - Z * focal
      + Y * screenH / 2.f
      - X * screenW / 2.f;
  screenX = X;
  screenY = Y;
  screenZ = Z;
  //!
  pixelW = static_cast<size_t>(scene->camera.imgWidth);
  pixelH = static_cast<size_t>(scene->camera.imgHeight);
  pixelRegion[0] = pixelRegion[1] = 0;
  pixelRegion[2] = pixelW;
  pixelRegion[3] = pixelH;
  pixelSize[0] = pixelRegion[2] - pixelRegion[0];
  pixelSize[1] = pixelRegion[3] - pixelRegion[1];
  //! frame-buffer
  renderImage->Init(static_cast<int>(pixelSize[0]),
                   static_cast<int>(pixelSize[1])); /* reinitialize local fb */
  colorBuffer = renderImage->GetPixels();
  depthBuffer = renderImage->GetZBuffer();
  sampleCountBuffer = renderImage->GetSampleCount();
  irradianceCountBuffer = renderImage->GetIrradianceComputationImage();
  maskBuffer = renderImage->GetMasks();
  //! multi-threading
  tileDimX = static_cast<size_t>(CEIL(static_cast<float>(pixelSize[0]) /
      static_cast<float>(tileSize)));
  tileDimY = static_cast<size_t>(CEIL(static_cast<float>(pixelSize[1]) /
      static_cast<float>(tileSize)));
  tileCount = tileDimX * tileDimY;
  //-------------------------------------------------------------------------//
  // Initialize Photon Map
  //-------------------------------------------------------------------------//
  // TODO: Photon Map for MPI Mode
#ifdef USE_GUI
  if (param.photonMapSize > 0) {
    scene->photonmap.AllocatePhotons
        (static_cast<qaUINT>(param.photonMapSize));
    scene->causticsmap.AllocatePhotons
        (static_cast<qaUINT>(param.causticsMapSize));
    //! find out all point lights
    std::vector<Light*> photonLights;
    std::vector<float>  photonValues(1,0.f);
    float totalValue = 0.f;
    for (auto &light : scene->lights) {
      if (light->IsPhotonSource()) {
        photonLights.push_back(light);
        photonValues.push_back(ColorLuma(light->GetPhotonIntensity()));
        totalValue += ColorLuma(light->GetPhotonIntensity());
      }
    }
    for (auto& v : photonValues) { v /= totalValue; }
    //! trace photons
    std::atomic<int> numPhotonsRec(0);
    std::atomic<int> numPhotonsGen(0);
    tasking::parallel_for
        (size_t(0), tasking::get_num_of_threads(), size_t(1),
         [&] (size_t k)
         {
           while (numPhotonsRec < param.photonMapSize)
           {
             float r;rng->local().Get1f(r);
             auto it = std::upper_bound(photonValues.begin(), photonValues.end(), r);
             long id = it - photonValues.begin();
             auto &light = photonLights[id-1];
             //! generate one photons
             ++numPhotonsGen;
             DiffRay ray = light->RandomPhoton(); ray.Normalize();
             DiffHitInfo hInfo;hInfo.c.z = BIGFLOAT;
             Color3f intensity = light->GetPhotonIntensity() / photonValues[id];
             //! trace photon
             size_t bounce = 0;
             while (bounce < param.photonMapBounce)
             {
               if (scene->TraceNodeNormal(scene->rootNode, ray, hInfo)) {
                 const Material *mtl = hInfo.c.node->GetMaterial();
                 if (mtl->RandomPhotonBounce(ray, intensity, hInfo))
                 {
                   ++bounce;
                 }
                 if (mtl->IsPhotonSurface(0))
                 {
                   scene->photonmap
                       .AddPhoton(cyPoint3f(hInfo.c.p.x, hInfo.c.p.y, hInfo.c.p.z),
                                  -cyPoint3f(ray.c.dir.x, ray.c.dir.y, ray.c.dir.z),
                                  cyColor(intensity.x, intensity.y, intensity.z));
                   ++numPhotonsRec;
                 }
                 else { bounce = param.photonMapBounce; }
               } else { bounce = param.photonMapBounce; }
             }
           }
         });
    scene->photonmap.ScalePhotonPowers(1.f / numPhotonsGen);
    scene->photonmap.PrepareForIrradianceEstimation();
    FILE *fp=fopen("photonmap.dat","wb");
    fwrite(scene->photonmap.GetPhotons(),sizeof(cyPhotonMap::Photon),
           scene->photonmap.NumPhotons(),fp);
    fclose(fp);
  }
#endif
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
  while (sampler.Loop())
  {
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
  const auto tileSta(static_cast<size_t>(mpiRank));
  const auto tileNum(static_cast<size_t>(tileCount));
  const auto tileStp(static_cast<size_t>(mpiSize));
  tasking::init();
  tasking::parallel_for(tileSta, tileNum, tileStp, [=](size_t k) {
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
      if (!threadStop) { PixelRender(i, j, k); }
    });
    renderImage->IncrementNumRenderPixel(static_cast<int>(numPixels));
  });
  //-------------------------------------------------------------------------//
  // Stop timing
  //-------------------------------------------------------------------------//
  StopTimer();
}
}