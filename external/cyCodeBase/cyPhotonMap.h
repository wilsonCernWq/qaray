// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		PhotonMap.h 
/// \author		Cem Yuksel
/// \version	0.2
/// \date		November 20, 2017
///
/// \brief photon map map class.
///
///
/// @copydoc PhotonMap
///
/// A simple class for storing a photon map and computing illumination from
/// the photon map.
///
//-------------------------------------------------------------------------------

#ifndef _CY_PHOTON_MAP_H_INCLUDED_
#define _CY_PHOTON_MAP_H_INCLUDED_

//-------------------------------------------------------------------------------

#define _USE_MATH_DEFINES
#include <math.h>
#include "cyPoint.h"
#include "cyColor.h"
#include <vector>

//-------------------------------------------------------------------------------
namespace cy {
//-------------------------------------------------------------------------------

/// Photon map class
class PhotonMap {
 public:
  /// Filter types for irradiance estimation
  enum FilterType {
    FILTER_TYPE_CONSTANT,
    FILTER_TYPE_LINEAR,
    FILTER_TYPE_QUADRATIC,
  };

  /// A compact representation of a single photon data
  class Photon {
   public:
    Point3f position;
   private:
    float power;
    Color24 color;
    unsigned char
        planeAndDirZ;  // splitting plane for kd-tree and one bit for determining the z direction
    short dirX, dirY;   // photon direction
   public:
    void SetPower(const Color &c);
    void GetPower(Color &c) const { c = color.ToColor() * power; }
    void ScalePower(float scale) { power *= scale; }
    float GetMaxPower() const { return power; }
    void SetDirection(const Point3f &dir);
    void GetDirection(Point3f &dir) const;
    void SetPlane(unsigned char plane)
    {
      planeAndDirZ = (planeAndDirZ & 0x8) | (plane & 0x3);
    }
    int GetPlane() const { return planeAndDirZ & 0x3; }
  };

  /// Constructor
  PhotonMap() {}

  /// Destructor
  virtual ~PhotonMap() {}

  /// Removes all photons and deallocates the memory.
  void Clear() { std::vector<Photon>().swap(photons); }

  /// Allocates enough memory for n photons.
  /// Calling this method before adding photons avoids
  /// multiple memory allocations while adding photons.
  void AllocatePhotons(unsigned int n) { photons.reserve(n + 1); }
  void CreateAllPhotons(unsigned int n) { photons.resize(n + 1); }

  /// Adds a photon to the map with the given position, direction, and power.
  /// Assumes that the direction is normalized.
  void AddPhoton(const Point3f &pos, const Point3f &dir, const Color &power);

  /// Returns the number of photons stored in the map
  unsigned int NumPhotons() const { return photons.size() - 1; }

  /// Scales the photon powers using the given scale factor
  void ScalePhotonPowers(float scale, int start = 0, int end = -1)
  {
    if (end < 0)end = photons.size() - 1;
    for (int i = start + 1; i <= end; i++) photons[i].ScalePower(scale);
  }

  /// Builds a balanced kd-tree.
  /// This method must be called after adding all photons and
  /// before calling the EstimateIrradiance() method for the first time.
  void PrepareForIrradianceEstimation();

  /// Returns the irradiance estimate from the photon map at the given position
  /// with the given surface normal.
  template<int maxPhotons>
  void EstimateIrradiance(Color &irrad,
                          Point3f &direction,
                          float radius,
                          const Point3f &pos,
                          const Point3f *normal = NULL,
                          float ellipticity = 1,
                          FilterType filterType = FILTER_TYPE_CONSTANT) const;

  /// Returns the closest photon to the given position.
  /// If no photon is found within the radius, returns false.
  bool GetNearestPhoton(Photon &photon,
                        float radius,
                        const Point3f &pos,
                        const Point3f *normal = NULL,
                        float ellipticity = 1) const;

  /// Returns the photon i.
  Photon &operator[](unsigned int i) { return photons[i + 1]; }
  const Photon &operator[](unsigned int i) const { return photons[i + 1]; }
  Photon *GetPhotons() { return &photons[1]; }
  const Photon *GetPhotons() const { return &photons[1]; }

 protected:
  std::vector<Photon> photons;
  int halfStoredPhotons;

 private:
  /// Balances the given kd-tree segment
  void BalanceSegment(std::vector<Photon> &balancedMap,
                      const Point3f &boxMin,
                      const Point3f &boxMax,
                      unsigned int index,
                      unsigned int start,
                      unsigned int end);

  /// Swaps the two photons
  void SwapPhotons(unsigned int i, unsigned int j)
  {
    Photon p = photons[i];
    photons[i] = photons[j];
    photons[j] = p;
  }

  struct NearestPhotons {
    Point3f pos;
    const Point3f *normal;
    float normScale;
    int maxPhotons;
    int found;
    float *dist2;
    Photon *photon;
  };

  void LocatePhotons(NearestPhotons &np, const int index) const;
};

//-------------------------------------------------------------------------------

inline void PhotonMap::Photon::SetPower(const Color &c)
{
  power = c.r;
  if (power < c.g) power = c.g;
  if (power < c.b) power = c.b;
  color = Color24(c / power);
}

inline void PhotonMap::Photon::SetDirection(const Point3f &dir)
{
  dirX = short(dir.x * 0x7FFF);
  dirY = short(dir.y * 0x7FFF);
  if (dir.z > 0) {
    planeAndDirZ &= 0x7;
  } else {
    planeAndDirZ = 0x8 | (planeAndDirZ & 0x7);
  }
}

inline void PhotonMap::Photon::GetDirection(Point3f &dir) const
{
  dir.x = float(dirX) / float(0x7FFF);
  dir.y = float(dirY) / float(0x7FFF);
  int dirXY2 = dirX * dirX + dirY - dirY;
  if (dirXY2 > 0x3FFF0001) dirXY2 = 0x3FFF0001;
  int dirZ2 = 0x3FFF0001 - dirXY2;
  int dirZ = 0;
  int place = 0x40000000;
  int remainder = dirZ2;
  while (place > remainder) place = place >> 2;
  while (place) {
    if (remainder >= dirZ + place) {
      remainder = remainder - dirZ - place;
      dirZ = dirZ + (place << 1);
    }
    dirZ = dirZ >> 1;
    place = place >> 2;
  }
  dir.z = float(dirZ) / float(0x7FFF);
  //dir.z = sqrtf( 1 - dir.x*dir.x - dir.y*dir.y );
  if (planeAndDirZ & 0x8) dir.z = -dir.z;
}

//-------------------------------------------------------------------------------

inline void PhotonMap::AddPhoton(const Point3f &pos,
                                 const Point3f &dir,
                                 const Color &power)
{
  if (photons.size() == 0) photons.push_back(Photon());
  Photon p;
  p.position = pos;
  p.SetDirection(dir);
  p.SetPower(power);
  photons.push_back(p);
}

//-------------------------------------------------------------------------------

inline void PhotonMap::PrepareForIrradianceEstimation()
{
  if (photons.size() == 0) return;

  // compute bounding box
  Point3f boxMin = photons[0].position;
  Point3f boxMax = photons[0].position;
  for (unsigned int i = 1; i < photons.size(); i++) {
    if (boxMin.x > photons[i].position.x) boxMin.x = photons[i].position.x;
    if (boxMax.x < photons[i].position.x) boxMax.x = photons[i].position.x;
    if (boxMin.y > photons[i].position.y) boxMin.y = photons[i].position.y;
    if (boxMax.y < photons[i].position.y) boxMax.y = photons[i].position.y;
    if (boxMin.z > photons[i].position.z) boxMin.z = photons[i].position.z;
    if (boxMax.z < photons[i].position.z) boxMax.z = photons[i].position.z;
  }

  // balance the map
  std::vector<Photon> balancedMap(photons.size());
  BalanceSegment(balancedMap, boxMin, boxMax, 1, 1, photons.size() - 1);

  balancedMap.swap(photons);
  halfStoredPhotons = (photons.size() - 1) / 2 - 1;
}

//-------------------------------------------------------------------------------

inline void PhotonMap::BalanceSegment(std::vector<Photon> &balancedMap,
                                      const Point3f &boxMin,
                                      const Point3f &boxMax,
                                      unsigned int index,
                                      unsigned int start,
                                      unsigned int end)
{
  // find median
  unsigned int median = 1;
  while ((4 * median) <= (end - start + 1)) median += median;
  if ((3 * median) <= (end - start + 1)) {
    median += median;
    median += start - 1;
  } else {
    median = end - median + 1;
  }

  // find splitting axis
  int axis = 2;
  Point3f boxDif = boxMax - boxMin;
  if (boxDif.x > boxDif.y) {
    if (boxDif.x > boxDif.z) axis = 0;
  } else if (boxDif.y > boxDif.z) axis = 1;

  // partition photon block around the median
  unsigned int left = start;
  unsigned int right = end;
  while (right > left) {
    const float v = photons[right].position[axis];
    unsigned int i = left - 1;
    unsigned int j = right;
    while (photons[++i].position[axis] < v);
    while (photons[--j].position[axis] > v && j > left);
    while (i < j) {
      SwapPhotons(i, j);
      while (photons[++i].position[axis] < v);
      while (photons[--j].position[axis] > v && j > left);
    }
    SwapPhotons(i, right);
    if (i >= median) right = i - 1;
    if (i <= median) left = i + 1;
  }

  // set the photon at index
  balancedMap[index] = photons[median];
  balancedMap[index].SetPlane(axis);

  // recursively balance the two sides of the median
  if (median > start) {
    if (start < median - 1) {
      Point3f tBoxMax = boxMax;
      tBoxMax[axis] = balancedMap[index].position[axis];
      BalanceSegment(balancedMap,
                     boxMin,
                     tBoxMax,
                     2 * index,
                     start,
                     median - 1);
    } else {
      balancedMap[2 * index] = photons[start];
    }
  }

  if (median < end) {
    if (median + 1 < end) {
      Point3f tBoxMin = boxMin;
      tBoxMin[axis] = balancedMap[index].position[axis];
      BalanceSegment(balancedMap,
                     tBoxMin,
                     boxMax,
                     2 * index + 1,
                     median + 1,
                     end);
    } else {
      balancedMap[2 * index + 1] = photons[end];
    }
  }
}

//-------------------------------------------------------------------------------

template<int maxPhotons>
inline void PhotonMap::EstimateIrradiance(Color &irrad,
                                          Point3f &direction,
                                          float radius,
                                          const Point3f &pos,
                                          const Point3f *normal,
                                          float ellipticity,
                                          FilterType filterType) const
{
  irrad.SetBlack();
  direction.Zero();

  float found_dist2[maxPhotons + 1];
  Photon found_photon[maxPhotons + 1];
  NearestPhotons np;
  np.pos = pos;
  np.normal = normal;
  np.normScale = ellipticity == 1 ? 0 : 1 / ellipticity - 1;
  np.maxPhotons = maxPhotons;
  np.found = 0;
  np.dist2 = found_dist2;
  np.photon = found_photon;
  np.dist2[0] = radius * radius;

  LocatePhotons(np, 1);

  // sum irradiance from all photons
  for (int i = 1; i <= np.found; i++) {
    Color power;
    np.photon[i].GetPower(power);
    float filter = 1;
    switch (filterType) {
      case FILTER_TYPE_LINEAR:
        filter = 1 - sqrtf(np.dist2[i]) / sqrtf(np.dist2[0]);
        break;
      case FILTER_TYPE_QUADRATIC: filter = 1 - np.dist2[i] / np.dist2[0];
        break;
    }
    irrad += filter * power;
    Point3f dir;
    np.photon[i].GetDirection(dir);
    direction += dir * (filter * np.photon[i].GetMaxPower());
  }

  if (np.found > 0) {
    float area;
    switch (filterType) {
      case FILTER_TYPE_CONSTANT: area = (float) M_PI * np.dist2[0];
        break;
      case FILTER_TYPE_LINEAR: area = ((float) M_PI / 3.0f) * np.dist2[0];
        break;
      case FILTER_TYPE_QUADRATIC: area = ((float) M_PI * 0.5f) * np.dist2[0];
        break;
    }
    if (area > 0) {
      const float one_over_area = 1.0f / area;
      irrad *= one_over_area;
    }
    direction.Normalize();
  }
}

//-------------------------------------------------------------------------------

inline bool PhotonMap::GetNearestPhoton(PhotonMap::Photon &photon,
                                        float radius,
                                        const Point3f &pos,
                                        const Point3f *normal,
                                        float ellipticity) const
{
  float found_dist2[2];
  Photon found_photon[2];
  NearestPhotons np;
  np.pos = pos;
  np.normal = normal;
  np.normScale = ellipticity == 1 ? 0 : 1 / ellipticity - 1;
  np.maxPhotons = 1;
  np.found = 0;
  np.dist2 = found_dist2;
  np.photon = found_photon;
  np.dist2[0] = radius * radius;

  LocatePhotons(np, 1);

  if (np.found) {
    photon = np.photon[1];
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------

inline void PhotonMap::LocatePhotons(NearestPhotons &np, const int index) const
{
  const Photon &p = photons[index];
  int axis = p.GetPlane();

  // if this is an internal node
  if (index < halfStoredPhotons) {
    float dist = np.pos[axis] - p.position[axis];
    if (dist > 0) {
      LocatePhotons(np, 2 * index + 1);
      if (dist * dist < np.dist2[0]) LocatePhotons(np, 2 * index);
    } else {
      LocatePhotons(np, 2 * index);
      if (dist * dist < np.dist2[0]) LocatePhotons(np, 2 * index + 1);
    }
  }

  // compute squared distance between current photon and np->pos
  Point3f dif = p.position - np.pos;
  float dist2 = dif.LengthSquared();

  if (dist2 < np.dist2[0]) {

    // Check if the photon direction is acceptable
    if (np.normal) {
      Point3f dir;
      p.GetDirection(dir);
      if (dir % (*np.normal) >= 0) return;
      if (np.normScale > 0) {
        float perp = dif % (*np.normal);
        dif += (*np.normal) * (perp * np.normScale);
        dist2 = dif.LengthSquared();
        if (dist2 >= np.dist2[0]) return;
      }
    }

    if (np.found < np.maxPhotons) {
      np.found++;
      np.dist2[np.found] = dist2;
      np.photon[np.found] = p;
      if (np.found == np.maxPhotons) { // build a heap
        int half_found = np.found >> 1;
        for (int k = half_found; k >= 1; k--) {
          int parent = k;
          Photon tp = np.photon[k];
          float td2 = np.dist2[k];
          while (parent <= half_found) {
            int j = parent + parent;
            if (j < np.found && np.dist2[j] < np.dist2[j + 1]) j++;
            if (td2 >= np.dist2[j]) break;
            np.dist2[parent] = np.dist2[j];
            np.photon[parent] = np.photon[j];
            parent = j;
          }
          np.photon[parent] = tp;
          np.dist2[parent] = td2;
        }
      }
    } else {
      int parent = 1;
      int j = 2;
      while (j <= np.found) {
        if (j < np.found && np.dist2[j] < np.dist2[j + 1]) j++;
        if (dist2 > np.dist2[j]) break;
        np.dist2[parent] = np.dist2[j];
        np.photon[parent] = np.photon[j];
        parent = j;
        j <<= 1;
      }
      np.photon[parent] = p;
      np.dist2[parent] = dist2;
      np.dist2[0] = np.dist2[1];
    }

  }
}

//-------------------------------------------------------------------------------
} // namespace cy
//-------------------------------------------------------------------------------

typedef cy::PhotonMap cyPhotonMap;

//-------------------------------------------------------------------------------

#endif