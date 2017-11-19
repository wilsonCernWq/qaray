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

static const Color air_absorption(0.001f, 0.001f, 0.001f);

const float total_reflection_threshold = 1.001f;
const float glossiness_value_threshold = 0.001f;
const float glossiness_angle_threshold = 0.001f;

//------------------------------------------------------------------------------

Color MtlBlinn_PathTracing::Shade (const DiffRay &ray,
                                   const DiffHitInfo &hInfo,
                                   const LightList &lights,
                                   int bounceCount)
const
{
  //
  // Calculate Ray Geometry
  const auto N = glm::normalize(hInfo.c.N);   // surface normal in world coordinate
  const auto V = glm::normalize(-ray.c.dir); // ray incoming direction
  const auto P = hInfo.c.p;                  // surface position in world coordinate
  //
  // Color Parameters
  Color color = hInfo.c.hasTexture ?
                emission.Sample(hInfo.c.uvw, hInfo.c.duvw) : emission.GetColor();
  const Color colorDiffuse = hInfo.c.hasTexture ?
                             diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) : diffuse.GetColor();
  const Color colorSpecular = hInfo.c.hasTexture ?
                              specular.Sample(hInfo.c.uvw, hInfo.c.duvw) : specular.GetColor();
  //
  // Direct Illumination
  const float coefNormalization = lights.size() == 0 ? 0.f : 1.f / lights.size();
  for (auto &light : lights)
  {
    if (light->IsAmbient()) { continue; }
    else
    {
      auto incoming = light->Illuminate(P, N) * coefNormalization;
      auto L = glm::normalize(-light->Direction(P));
      auto H = glm::normalize(V + L);
      auto cosNL = MAX(0.f, glm::dot(N, L));
      auto cosNH = MAX(0.f, glm::dot(N, H));
      color += (colorDiffuse * cosNL + colorSpecular * POW(cosNH, specularGlossiness)) * incoming;
    }
  }

  if (bounceCount > 0)
  {
    //
    // Ray Differential
    const auto Vx = glm::normalize(-ray.x.dir); // diff ray incoming direction
    const auto Vy = glm::normalize(-ray.y.dir); // diff ray incoming direction
    const auto Px = ray.x.p + ray.x.dir * hInfo.x.z;
    const auto Py = ray.y.p + ray.y.dir * hInfo.y.z;
    //
    // Index of Refraction
    const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
    float cosI = CLAMP(glm::dot(N, V), 0.f, 1.f);
    float sinI = SQRT(1 - cosI * cosI);
    assert(cosI <= 1.f);
    assert(sinI <= 1.f);
    //
    // Reflection and Transmission coefficients
    const bool totalReflection = (nIOR * sinI) > total_reflection_threshold;
    const float c0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
    const float cR = c0 + (1.f - c0) * POW(1.f - ABS(cosI), 5.f);
    const float cT = 1.f - cR;
    assert(cR <= 1.f);
    assert(cT <= 1.f);
    const Color kR = hInfo.c.hasTexture ?
                     reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
    const Color kT = hInfo.c.hasTexture ?
                     refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
    //
    // Coefficients
    const Color colorRefraction = totalReflection ? Color(0.f) : kT * cT;
    const Color colorReflection = totalReflection ? (kR + kT) : (kR + kT * cR);
    const float weightDiffuse = ColorMax(colorDiffuse);
    const float weightSpecular = ColorMax(colorSpecular);
    const float weightRefraction = ColorMax(colorRefraction);
    const float weightReflection = ColorMax(colorReflection);
    const float weightTotal = weightDiffuse + weightSpecular + weightRefraction + weightReflection;
    const float coefDiffuse = weightDiffuse / weightTotal;
    const float coefSpecular = weightSpecular / weightTotal;
    const float coefRefraction = weightRefraction / weightTotal;
    const float coefReflection = weightReflection / weightTotal;
    // debug(coefDiffuse);
    // debug(coefSpecular);
    // debug(coefReflection);
    // debug(coefRefraction);
    //
    // Indirect Illumination
    const float selection = rng->Get();
    if (selection < coefDiffuse) /* Sample Diffuse */
    {
      //
      //-- Sampling Cos Weighted Hemisphere
      //-- Method 1
      const float r1 = rng->Get(); // cosTheta
      const float r2 = rng->Get();
      //-- Method 2 (the idx_halton can go out of limit)
      // const float r1 = Halton(idx_halton,2);
      // const float r2 = Halton(idx_halton,3);
      // ++idx_halton;
      //
      // Generate random direction on unit hemisphere proportional to cosine-weighted solid angle
      // PDF = cos(theta) / PI
      //
      Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
      //
      //-- Compute local coordinate frame
      Point3 new_x, new_y, new_z = N;
      if (ABS(new_z.x) > ABS(new_z.y))
      {
        new_y = glm::normalize(Point3(new_z.z, 0, -new_z.x));
      } else
      {
        new_y = glm::normalize(Point3(0, -new_z.z, new_z.y));
      }
      new_x = glm::normalize(glm::cross(new_y, new_z));
      //
      //-- Generate ray
      const Point3 sample_dir = sample.x * new_x + sample.y * new_y + sample.z * new_z;
      DiffRay sample_ray(P, sample_dir);
      sample_ray.Normalize();
      DiffHitInfo sample_hInfo;
      sample_hInfo.c.z = BIGFLOAT;
      //
      //-- Incoming light radiation
      Color incoming;
      if (TraceNodeNormal(rootNode, sample_ray, sample_hInfo))
      {
        incoming =
            sample_hInfo.c.node->GetMaterial()->Shade(sample_ray, sample_hInfo, lights, bounceCount - 1);
      } else
      {
        incoming = environment.SampleEnvironment(sample_ray.c.dir);
      }
      //
      //-- BRDF
      const Point3 BRDF = colorDiffuse;
      //
      //-- Outgoing
      color += incoming * BRDF * 2.f / coefDiffuse;
    } else if (selection < coefSpecular + coefDiffuse) /* Sample Specular */
    {
      //
      //-- Sampling Cos Weighted Hemisphere
      //-- Method 1
      const float r1 = rng->Get(); // cosTheta
      const float r2 = rng->Get();
      //-- Method 2 (the idx_halton can go out of limit)
      // const float r1 = Halton(idx_halton,2);
      // const float r2 = Halton(idx_halton,3);
      // ++idx_halton;
      //
      // Generate random direction on unit hemisphere proportional to cosine-weighted solid angle
      // PDF = cos(theta) / PI
      //
      Point3 sample = glm::normalize(CosWeightedSampleHemiSphere(r1, r2));
      //
      //-- Compute local coordinate frame
      Point3 new_x, new_y, new_z = N;
      if (ABS(new_z.x) > ABS(new_z.y))
      {
        new_y = glm::normalize(Point3(new_z.z, 0, -new_z.x));
      } else
      {
        new_y = glm::normalize(Point3(0, -new_z.z, new_z.y));
      }
      new_x = glm::normalize(glm::cross(new_y, new_z));
      //
      //-- Generate ray
      const Point3 sample_dir = sample.x * new_x + sample.y * new_y + sample.z * new_z;
      DiffRay sample_ray(P, sample_dir);
      sample_ray.Normalize();
      DiffHitInfo sample_hInfo;
      sample_hInfo.c.z = BIGFLOAT;
      //
      //-- Incoming light radiation
      Color incoming;
      if (TraceNodeNormal(rootNode, sample_ray, sample_hInfo))
      {
        incoming =
            sample_hInfo.c.node->GetMaterial()->Shade(sample_ray, sample_hInfo, lights, bounceCount - 1);
      } else
      {
        incoming = environment.SampleEnvironment(sample_ray.c.dir);
      }
      //
      //-- BRDF
      const auto H = glm::normalize(V + sample_dir);
      const auto cosNH = MAX(0.f, glm::dot(N, H));
      const Point3 BRDF = colorSpecular * POW(cosNH, specularGlossiness);
      //
      //-- Outgoing
      color += incoming * BRDF * 2.f / coefSpecular;
    } else /* Sample Reflection & Reflraction */
    {
      //
      // Calculate Coordinate Frame
      const auto X = glm::normalize(glm::cross(glm::cross(N, V), N)); // Vx
      const auto Y = glm::normalize(N * glm::dot(N, V));              // Vy
      //
      // Refraction and Reflection Angle
      Point3 tDir, rDir, txDir, rxDir, tyDir, ryDir;
      do
      {
        //
        // Jittered Normal
        Point3 tjN = N, rjN = N;
        if (refractionGlossiness > glossiness_value_threshold)
        {
          const float r1 = rng->Get();
          const float r2 = rng->Get();
          const float r3 = rng->Get();
          tjN = glm::normalize(N + GetCirclePoint(r1, r2, r3, refractionGlossiness));
        }
        if (reflectionGlossiness > glossiness_value_threshold)
        {
          const float r1 = rng->Get();
          const float r2 = rng->Get();
          const float r3 = rng->Get();
          rjN = glm::normalize(N + GetCirclePoint(r1, r2, r3, refractionGlossiness));
        }
        //
        // Incidence & Refraction Angle
        cosI = glm::dot(tjN, V);
        sinI = SQRT(1 - cosI * cosI);
        const float cosIx = glm::dot(tjN, Vx);
        const float sinIx = SQRT(1 - cosIx * cosIx);
        const float cosIy = glm::dot(tjN, Vy);
        const float sinIy = SQRT(1 - cosIy * cosIy);
        const float sinO = MAX(0.f, MIN(1.f, sinI * nIOR));
        const float cosO = SQRT(1.f - sinO * sinO);
        const float sinOx = MAX(0.f, MIN(1.f, sinIx * nIOR));
        const float cosOx = SQRT(1.f - sinOx * sinOx);
        const float sinOy = MAX(0.f, MIN(1.f, sinIy * nIOR));
        const float cosOy = SQRT(1.f - sinOy * sinOy);
        //
        // Ray Directions
        tDir = -X * sinO - Y * cosO;
        txDir = -X * sinOx - Y * cosOx;
        tyDir = -X * sinOy - Y * cosOy;
        rDir = 2.f * rjN * (glm::dot(rjN, V)) - V;
        rxDir = 2.f * rjN * (glm::dot(rjN, Vx)) - Vx;
        ryDir = 2.f * rjN * (glm::dot(rjN, Vy)) - Vy;
        //
        // Loop Early Termination
        if (refractionGlossiness > glossiness_value_threshold ||
            reflectionGlossiness > glossiness_value_threshold) { break; }
      } while ((glm::dot(tDir, Y) > glossiness_angle_threshold) ||
               (glm::dot(txDir, Y) > glossiness_angle_threshold) ||
               (glm::dot(tyDir, Y) > glossiness_angle_threshold) ||
               (glm::dot(rDir, Y) < -glossiness_angle_threshold) ||
               (glm::dot(rxDir, Y) < -glossiness_angle_threshold) ||
               (glm::dot(ryDir, Y) < -glossiness_angle_threshold));
      //
      //-- Refraction
      {
        //
        //-- Generate ray
        DiffRay sample_ray(P, tDir, Px, txDir, Py, tyDir);
        sample_ray.Normalize();
        DiffHitInfo sample_hInfo;
        sample_hInfo.c.z = BIGFLOAT;
        //
        //-- Incoming light radiation
        Color incoming;
        if (TraceNodeNormal(rootNode, sample_ray, sample_hInfo))
        {
          auto *mtl = sample_hInfo.c.node->GetMaterial();
          incoming =
              (sample_hInfo.c.hasFrontHit ?
               Color(1.0f) :
               Attenuation(absorption, sample_hInfo.c.z)) *
              mtl->Shade(sample_ray, sample_hInfo, lights, bounceCount - 1);
        } else
        {
          incoming = environment.SampleEnvironment(sample_ray.c.dir);
        }
        //
        //-- BRDF
        const Point3 BRDF = colorRefraction;
        //
        //-- Outgoing
        color += incoming * BRDF * 2.f / (coefRefraction + coefRefraction);
      }
      //
      //-- Reflection
      {
        //
        //-- Generate ray
        DiffRay sample_ray(P, rDir, Px, rxDir, Py, ryDir);
        sample_ray.Normalize();
        DiffHitInfo sample_hInfo;
        sample_hInfo.c.z = BIGFLOAT;
        //
        //-- Incoming light radiation
        Color incoming;
        if (TraceNodeNormal(rootNode, sample_ray, sample_hInfo))
        {
          auto *mtl = sample_hInfo.c.node->GetMaterial();
          incoming =
              (sample_hInfo.c.hasFrontHit ?
               Color(1.0f) :
               Attenuation(absorption, sample_hInfo.c.z)) *
              mtl->Shade(sample_ray, sample_hInfo, lights, bounceCount - 1);
        } else
        {
          incoming = environment.SampleEnvironment(sample_ray.c.dir);
        }
        //
        //-- BRDF
        const Point3 BRDF = colorReflection;
        //
        //-- Outgoing
        color += incoming * BRDF * 2.f / (coefRefraction + coefRefraction);
      }
    }
  }
  //
  //-- Post Process Color
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
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
  return color;
}

//------------------------------------------------------------------------------
