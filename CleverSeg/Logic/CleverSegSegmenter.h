#ifndef FAST_MARCHING_H
#define FAST_MARCHING_H

#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <iterator>

#include "Utilities.h"

namespace CS {

const float  DIST_INF = 1.0;
const float  DIST_EPSION = 0.0;
unsigned char NNGBH = 26;
typedef float FPixelType;

template<typename SrcPixelType, typename LabPixelType>
class CleverSeg {
public:
    CleverSeg();
    ~CleverSeg();

    void SetSourceImage(const std::vector<SrcPixelType>& imSrc);
    void SetSeedlImage(std::vector<LabPixelType>& imSeed);
    void SetWorkMode(bool bSegInitialized = false);
    void SetImageSize(const std::vector<long>& imSize);
    void DoSegmentation();
    void GetLabeImage(std::vector<LabPixelType>& imLab);
    void GetForegroundmage(std::vector<LabPixelType>& imFgrd);

private:
    void calculateNeighboursAndInitialWeights();
    void CleverSegDefault();


    std::vector<SrcPixelType> m_imSrc;
    std::vector<LabPixelType> m_imSeed;
    std::vector<LabPixelType> m_imLabPre;
    std::vector<FPixelType> m_imDistPre;
    std::vector<LabPixelType> m_imLab;
    std::vector<FPixelType> m_imDist;

    std::vector<long> m_imSize;
    long m_DIMX, m_DIMY, m_DIMZ, m_DIMXY, m_DIMXYZ;
    std::vector<int> m_indOff;
    std::vector<unsigned char>  m_NBSIZE;
	float maxC;

 
    bool m_bSegInitialized;
};

}

#include "CleverSegSegmenter.hxx"

#endif
