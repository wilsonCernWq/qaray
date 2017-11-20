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
  Color color = hInfo.c.hasTexture ?
                emission.Sample(hInfo.c.uvw, hInfo.c.duvw) : emission.GetColor();
  const auto N  = glm::normalize(hInfo.c.N);  // surface normal in world coordinate
  const auto V  = glm::normalize(-ray.c.dir); // ray incoming direction
  const auto Vx = glm::normalize(-ray.x.dir); // diff ray incoming direction
  const auto Vy = glm::normalize(-ray.y.dir); // diff ray incoming direction
  const auto p  = hInfo.c.p;                  // surface position in world coordinate
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;
  //
  // Local Coordinate Frame
  const auto Y = glm::dot(N, V) > 0.f ? N : -N;    // Vy
  const auto Z = glm::cross(V, Y);
  const auto X = glm::normalize(glm::cross(Y, Z)); // Vx
  //
  // Index of Refraction
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
  const float cosI  = glm::dot(N, V);
  const float sinI  = SQRT(1 - cosI * cosI);
  const float sinO  = MAX(0.f, MIN(1.f, sinI * nIOR));
  const float cosO  = SQRT(1.f - sinO * sinO);
  const Point3 tDir = -X * sinO - Y * cosO;           // Transmission
  const Point3 rDir = 2.f * N * (glm::dot(N, V)) - V; // Reflection
  //
  // Reflection and Transmission coefficients
  const float C0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  //
  // Reflection and Transmission Colors
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
  float coefRefraction = ColorMax(sampleRefraction);
  float coefReflection = ColorMax(sampleReflection);
  float coefSpecular   = ColorMax(sampleSpecular);
  float coefDiffuse    = ColorMax(sampleDiffuse);
  const float coefSum = coefRefraction + coefReflection + coefSpecular + coefDiffuse;
  coefRefraction /= coefSum;
  coefReflection /= coefSum;
  coefSpecular   /= coefSum;
  coefDiffuse    /= coefSum;
  //
  // Shading Directional Lights  
  if (hInfo.c.hasFrontHit) 
  {
    const float normCoefDI = 1.f;
    //const float normCoefDI = lights.size() == 0 ? 1.f : 1.f / lights.size();
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
  if (bounceCount > 0)
  {
    //
    // Coordinate Frame for the Hemisphere
    Point3 new_x, new_y, new_z = Y;
    if (ABS(new_z.x) > ABS(new_z.y))
    {
      new_y = glm::normalize(Point3(new_z.z, 0, -new_z.x));
    }
    else
    {
      new_y = glm::normalize(Point3(0, -new_z.z, new_z.y));
    }
    new_x = glm::normalize(glm::cross(new_y, new_z));
    Point3 sample_dir(0.f);
    float pdf = 0.25f / (float)M_PI;
    //
    // Select a BxDF
    Color BxDF(0.f);
    const float select = rng->Get();
    /* Refraction */ 
    if (select < coefRefraction && coefRefraction > 1e-6f)
    {
      //
      // Generate Sample
      if (refractionGlossiness > glossiness_power_threshold) 
      {
	/* Random Sampling for Glossy Surface */
	Point3 sample = 
	  glm::normalize(CosWeightedSampleHemiSphere(rng->Get(), rng->Get()));      
	sample_dir = -(sample.x * new_x + sample.y * new_y + sample.z * new_z);
	pdf = coefRefraction * 2.f;
      } 
      else 
      {
	sample_dir = tDir;
	pdf = coefRefraction;
      }
      //
      // BSDF
      if (refractionGlossiness > glossiness_power_threshold) 
      {
	const Point3 L = glm::normalize(sample_dir);
	const Point3 H = tDir;
	const float cosNL = MAX(0.f, glm::dot(N, L));	
	const float cosNH = MAX(0.f, glm::dot(N, H));
	const float glossiness = POW(cosNH, refractionGlossiness); // My Hack
	BxDF = sampleRefraction * glossiness;
      } 
      else 
      {
	BxDF = sampleRefraction;
      }
    }
    else
    {
      /* Reflection */ 
      if (select < coefRefraction + coefReflection && coefReflection > 1e-6f) 
      {
	// Generate Sample
	if (reflectionGlossiness > glossiness_power_threshold)
	{
	  /* Random Sampling for Glossy Surface */
	  Point3 sample = 
	    glm::normalize(CosWeightedSampleHemiSphere(rng->Get(), rng->Get()));      
	  sample_dir = sample.x * new_x + sample.y * new_y + sample.z * new_z;
	  pdf = coefReflection * 2.f;
	} 
	else 
	{ 
	  sample_dir = rDir; 
	  pdf = coefReflection;
	}
	// BRDF
	if (reflectionGlossiness > glossiness_power_threshold) 
	{
	  const Point3 L = glm::normalize(sample_dir);
	  const Point3 H = glm::normalize(V + L);
	  const float cosNH = MAX(0.f, glm::dot(N, H));
	  BxDF = sampleReflection * POW(cosNH, reflectionGlossiness);
	} else {
	  BxDF = sampleReflection;
	}
      }
      /* Specular */
      else if (select < coefRefraction + coefReflection + coefSpecular &&
	       coefSpecular > 1e-6f) 
      {
	// Generate Sample
	if (specularGlossiness > glossiness_power_threshold)
	{
	  /* Random Sampling for Glossy Surface */
	  Point3 sample = 
	    glm::normalize(CosWeightedSampleHemiSphere(rng->Get(), rng->Get()));      
	  sample_dir = sample.x * new_x + sample.y * new_y + sample.z * new_z;
	  pdf = coefSpecular * 2.f;
	} 
	else 
	{ 
	  sample_dir = rDir; 
	  pdf = coefSpecular;
	}
	// BRDF
	if (hInfo.c.hasFrontHit)
	{
	  const Point3 L = glm::normalize(sample_dir);
	  const Point3 H = glm::normalize(V + L);
	  const float cosNH = MAX(0.f, glm::dot(N, H));
	  BxDF = sampleSpecular * POW(cosNH, specularGlossiness);
	}
      }
      else /* Diffuse */
      {
	// Generate Sample
	Point3 sample = 
	  glm::normalize(CosWeightedSampleHemiSphere(rng->Get(), rng->Get()));      
	sample_dir = sample.x * new_x + sample.y * new_y + sample.z * new_z;
	pdf = coefDiffuse * 2.f;	  
	// BRDF	
	if (hInfo.c.hasFrontHit) { BxDF = sampleDiffuse; }
      }
    }    
    //
    // Generate ray
    DiffRay sample_ray(p, sample_dir);
    DiffHitInfo sample_hInfo;
    sample_hInfo.c.z = BIGFLOAT;
    sample_ray.Normalize();          
    //
    // Integrate Incoming Ray
    Color incoming(0.f);
    if (TraceNodeNormal(rootNode, sample_ray, sample_hInfo))
    {      
      // Attenuation When the Ray Travels Inside the Material
      if (!sample_hInfo.c.hasFrontHit) { 
	incoming *= Attenuation(absorption, sample_hInfo.c.z);
      }
      const auto *mtl = sample_hInfo.c.node->GetMaterial();
      incoming = mtl->Shade(sample_ray, sample_hInfo, lights, bounceCount - 1);
    }
    else
    {
      incoming = environment.SampleEnvironment(sample_ray.c.dir);
    }
    Point3 outgoing = incoming * BxDF / pdf;
    //
    // Attenuate Outgoing Ray
    if (hInfo.c.hasFrontHit) 
    {     
      // TODO making volume effect here
    }
    color += outgoing;
  }  
  //
  //-- Post Process Color
  if (Material::sRGB)
  {
    color.r = LinearToSRGB(color.r);
    color.g = LinearToSRGB(color.g);
    color.b = LinearToSRGB(color.b);
  }
  if (Material::gamma != 1.f)
  {
    color.r = POW(color.r, 1.f / Material::gamma);
    color.g = POW(color.g, 1.f / Material::gamma);
    color.b = POW(color.b, 1.f / Material::gamma);
  }
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
  return color;
}

//------------------------------------------------------------------------------
