#include "globalvar.h"
#include "MtlBlinn_PathTracing.h"

//------------------------------------------------------------------------------

MtlBlinn_PathTracing::MtlBlinn_PathTracing () :
    diffuse(0.5f, 0.5f, 0.5f),
    specular(0.7f, 0.7f, 0.7f),
    emission(0, 0, 0),
    reflection(0, 0, 0),
    refraction(0, 0, 0),
    absorption(0, 0, 0),
    ior(1),
    specularGlossiness(20.0f),
    reflectionGlossiness(0),
    refractionGlossiness(0) {}

//------------------------------------------------------------------------------

float ColorMax (const Color &c) { return MAX(c.x, MAX(c.y, c.z)); }

void GetRandomSamples(const DiffHitInfo& hInfo, float& r1, float& r2)
{
  r1 = rng->Get();
  r2 = rng->Get();
  //TBBHaltonRNG.local().Get(r1, r2);
}

//------------------------------------------------------------------------------

const float total_reflection_threshold = 1.001f;
const float glossiness_value_threshold = 0.001f;
const float glossiness_power_threshold = 0.f;

//------------------------------------------------------------------------------

void MtlBlinn_PathTracing::SetReflectionGlossiness (float gloss)
{
  reflectionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
}

void MtlBlinn_PathTracing::SetRefractionGlossiness (float gloss)
{
  refractionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
}

Color MtlBlinn_PathTracing::Shade (const DiffRay &ray,
                                   const DiffHitInfo &hInfo,
                                   const LightList &lights,
                                   int bounceCount)
  const
{
  //
  // Differential Geometry
  //
  Color color = hInfo.c.hasTexture ?
                emission.Sample(hInfo.c.uvw, hInfo.c.duvw) :
                emission.GetColor();
  // Surface Normal In World Coordinate
  // Ray Incoming Direction
  // X Differential Ray Incoming Direction
  // Y Differential Ray Incoming Direction
  // Durface Position in World Coordinate
  const auto N = glm::normalize(hInfo.c.N);
  const auto V = glm::normalize(-ray.c.dir);
  const auto Vx = glm::normalize(-ray.x.dir);
  const auto Vy = glm::normalize(-ray.y.dir);
  const auto p  = hInfo.c.p;
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;
  //
  // Local Coordinate Frame
  //
  const auto Y = glm::dot(N, V) > 0.f ? N : -N;    // Vy
  const auto Z = glm::cross(V, Y);
  const auto X = glm::normalize(glm::cross(Y, Z)); // Vx
  //
  // Index of Refraction
  //
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
  const float cosI = glm::dot(N, V);
  const float sinI = SQRT(1 - cosI * cosI);
  const float sinO = MAX(0.f, MIN(1.f, sinI * nIOR));
  const float cosO = SQRT(1.f - sinO * sinO);
  const Point3 tDir = -X * sinO - Y * cosO;           // Transmission
  const Point3 rDir = 2.f * N * (glm::dot(N, V)) - V; // Reflection
  //
  // Reflection and Transmission coefficients
  //
  const float C0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  //
  // Reflection and Transmission Colors
  //
  const bool totReflection = (nIOR * sinI) > total_reflection_threshold;
  const Color tK =
      hInfo.c.hasTexture ?
      refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color rK =
      hInfo.c.hasTexture ?
      reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const Color sampleRefraction = totReflection ? Color(0.f) : tK * tC;
  const Color sampleReflection = totReflection ? (rK + tK) : (rK + tK * rC);
  const Color sampleDiffuse =
      hInfo.c.hasTexture ?
      diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      diffuse.GetColor();
  const Color sampleSpecular =
      hInfo.c.hasTexture ?
      specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      specular.GetColor();
  //
  // Throw A Dice
  //
  const float select = rng->Get();
  float coefDirectLight = 0.f;
  float coefRefraction = ColorMax(sampleRefraction);
  float coefReflection = ColorMax(sampleReflection);
  float coefSpecular = ColorMax(sampleSpecular);
  float coefDiffuse = ColorMax(sampleDiffuse);
  const float coefSum =
      coefDirectLight + coefRefraction + coefReflection + coefSpecular + coefDiffuse;
  coefDirectLight /= coefSum;
  coefRefraction  /= coefSum;
  coefReflection  /= coefSum;
  coefSpecular    /= coefSum;
  coefDiffuse     /= coefSum;
  const float sumDirectLight = coefDirectLight;
  const float sumRefraction = coefRefraction + coefDirectLight;
  const float sumReflection = coefReflection + coefRefraction + coefDirectLight;
  const float sumSpecular = coefSpecular + coefReflection + coefRefraction + coefDirectLight;
  const float sumDiffuse = 1.00001f; /* make sure everything is inside the range */
  //
  // Shading Directional Lights
  //
  //if (bounceCount == 0 || select <= sumDirectLight)
  {
    // const float normCoefDI = 
    //   (bounceCount == 0 ? 1.f : sumDirectLight) / 
    //   (lights.size() == 0 ? 1.f : lights.size()) * (float)M_PI;
    const float normCoefDI = (lights.size() == 0 ? 1.f : 1.f / lights.size()) / (float)M_PI;
    for (auto &light : lights)
    {
      if (light->IsAmbient()) {}
      else
      {
	auto intensity = light->Illuminate(p, N) * normCoefDI;
	auto L = glm::normalize(-light->Direction(p));
	auto H = glm::normalize(V + L);
	auto cosNL = MAX(0.f, glm::dot(N, L));
	auto cosNH = MAX(0.f, glm::dot(N, H));
	color += normCoefDI * intensity * cosNL * 
	  (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
      }
    }
  }
  //
  // Shading Indirectional Lights
  //
  /*else*/ if (bounceCount > 0)
  {
    //
    // Coordinate Frame for the Hemisphere
    const Point3 nZ = Y;
    const Point3 nY = (ABS(nZ.x) > ABS(nZ.y)) ?
      glm::normalize(Point3(nZ.z, 0, -nZ.x)) :
      glm::normalize(Point3(0, -nZ.z, nZ.y));
    const Point3 nX = glm::normalize(glm::cross(nY, nZ));
    Point3 sampleDir(0.f);
    bool doShade = false;
    //
    // Select a BxDF
    float PDF = 1.f;
    Color BxDF(0.f);
    /* Refraction */
    if (select <= sumRefraction && coefRefraction > 1e-6f)
    {
      if (refractionGlossiness > glossiness_power_threshold) {
        /* Random Sampling for Glossy Surface */
	float r1, r2;
	GetRandomSamples(hInfo, r1, r2);
        const Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
        sampleDir = -(sample.x * nX + sample.y * nY + sample.z * nZ);
	/* PDF */
        PDF = coefRefraction / (float) M_PI;
	/* BSDF */
	const Point3 L = glm::normalize(sampleDir);
	const Point3 H = tDir;
        const float cosVH = MAX(0.f, glm::dot(V, H));
        const float glossiness = POW(cosVH, refractionGlossiness); // My Hack
        BxDF = sampleRefraction * glossiness / (float)M_PI;
      } else {
        /* Ray Direction */
        sampleDir = tDir;
	/* PDF */
        PDF = coefRefraction;
	/* BSDF */
        BxDF = sampleRefraction / (float)M_PI;
      }
      doShade = true;
    } 
    /* Reflection */
    else if (select < sumReflection && coefReflection > 1e-6f)
    {           
      if (reflectionGlossiness > glossiness_power_threshold)
      {
	/* Random Sampling for Glossy Surface */
	float r1, r2;
	GetRandomSamples(hInfo, r1, r2);
        const Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
        sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
	/* PDF */
	PDF = coefReflection / (float) M_PI;
	/* BRDF */
	const Point3 L = glm::normalize(sampleDir);
	const Point3 H = rDir;
	const float cosVH = MAX(0.f, glm::dot(V, H));
	BxDF = sampleReflection * POW(cosVH, reflectionGlossiness) / (float)M_PI;
      } else {
        /* Ray Direction */
	sampleDir = rDir;
	/* PDF */
	PDF = coefReflection;
	/* BRDF */
	BxDF = sampleReflection / (float)M_PI;
      }
      doShade = true;
    }
    /* Specular */
    else if (select < sumSpecular && coefSpecular > 1e-6f)
    {
      if (specularGlossiness > glossiness_power_threshold) 
      {
	if (hInfo.c.hasFrontHit) 
	{
	  /* Random Sampling for Glossy Surface */
	  float r1, r2;
	  GetRandomSamples(hInfo, r1, r2);
	  const Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
	  sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
	  /* PDF */
	  PDF = coefSpecular / (float) M_PI;
	  /* BRDF */
	  const Point3 L = glm::normalize(sampleDir);
	  const Point3 H = glm::normalize(V + L);
	  const float cosNH = MAX(0.f, glm::dot(N, H));
	  BxDF = sampleSpecular * POW(cosNH, specularGlossiness) / (float)M_PI;	
	  doShade = true;
	}
      }
    }
    /* Diffuse */
    else if (coefDiffuse > 1e-6f)
    {
      if (hInfo.c.hasFrontHit) 
      {
	/* Generate Random Sample */
	float r1, r2;
	GetRandomSamples(hInfo, r1, r2);
	const Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
	sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
	/* PDF */
	PDF = coefDiffuse / (float) M_PI;
	/* BRDF */
	BxDF = sampleDiffuse / (float)M_PI; 
	doShade = true;
      }
    }    
    //
    // Shade
    if (doShade)
    {
      // Generate ray
      DiffRay sampleRay(p, sampleDir);
      DiffHitInfo sampleHInfo;
      sampleHInfo.c.z = BIGFLOAT;
      sampleHInfo.c.haltonRNG = hInfo.c.haltonRNG;
      sampleRay.Normalize();
      // Integrate Incoming Ray
      Color incoming(0.f);
      if (TraceNodeNormal(rootNode, sampleRay, sampleHInfo))
      {
        // Attenuation When the Ray Travels Inside the Material
        if (!sampleHInfo.c.hasFrontHit)
        {
          incoming *= Attenuation(absorption, sampleHInfo.c.z);
        }
        const auto *mtl = sampleHInfo.c.node->GetMaterial();
        incoming = mtl->Shade(sampleRay, sampleHInfo, lights, bounceCount - 1);
      } else
      {
        incoming = environment.SampleEnvironment(sampleRay.c.dir);
      }
      Point3 outgoing = incoming * BxDF / PDF;
      // Attenuate Outgoing Ray
      if (hInfo.c.hasFrontHit)
      {
        // TODO making volume effect here
	//outgoing *= MIN(1.f, 1.f / (0.0025f * hInfo.c.z * hInfo.c.z));
      }
      color += outgoing;
    }
  }
  return color;
}

//------------------------------------------------------------------------------
