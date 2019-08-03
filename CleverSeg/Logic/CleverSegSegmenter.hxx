
#include "CleverSegSegmenter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkNumericTraits.h"
#include "itkRegionOfInterestImageFilter.h"

namespace CS {

	template<typename SrcPixelType, typename LabPixelType>
	CleverSeg<SrcPixelType, LabPixelType>
		::CleverSeg() {
		m_bSegInitialized = false;
	}

	template<typename SrcPixelType, typename LabPixelType>
	CleverSeg<SrcPixelType, LabPixelType>
		::~CleverSeg() {
		std::cout << "CleverSeg destroyed\n";
	}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::SetSourceImage(const std::vector<SrcPixelType>& imSrc) {

    m_imSrc = imSrc;
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::SetSeedlImage(std::vector<LabPixelType>& imSeed) {

    m_imSeed = imSeed;
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::SetWorkMode(bool bSegUnInitialized ) {

    m_bSegInitialized = bSegUnInitialized;
}
template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::SetImageSize(const std::vector<long>& imSize) {

    m_imSize = imSize;
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::calculateNeighboursAndInitialWeights() {
	long  i, j, k, index = 0;
	maxC = 0;
	m_DIMX = m_imSize[0];
	m_DIMY = m_imSize[1];
	m_DIMZ = m_imSize[2];
	m_DIMXY = m_DIMX * m_DIMY;
	m_DIMXYZ = m_DIMXY * (m_DIMZ == 0 ? 1 : m_DIMZ);
	m_imLab.resize(m_DIMXYZ);
	m_imDist.resize(m_DIMXYZ);
	m_imLabPre.resize(m_DIMXYZ);
	m_imDistPre.resize(m_DIMXYZ);


	// Determine neighborhood size at each vertice and Initilize seeds
	for (k = -1; k <= 1; k++)
		for (i = -1; i <= 1; i++)
			for (j = -1; j <= 1; j++)
				if (!(i == 0 && j == 0 && k == 0))
					m_indOff.push_back( i + j * (int) m_DIMX + k * ( (int) m_DIMX * (int) m_DIMY ) );

	
	// Initialize weights
	for (index = 0; index < m_DIMXYZ; index++) {
		m_imLab[index] = m_imSeed[index];
		if (m_imSrc[index] > maxC)
			maxC = m_imSrc[index];
		m_imLab[index] == 0 ? m_imDist[index] = DIST_EPSION : m_imDist[index] = DIST_INF;
	}
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::CleverSegDefault() {
	
    float theta, C, g;
    long idxCenter, i, j, k, cont = 0, maxIt = 999999;
	float myDiff = 0;
	long zinit, zend, idxNgbh;
	bool converged = false;

	while (!converged) {
		converged = true;
		for (k = 1; k < m_DIMZ; k++) {

			for (j = 1; j < m_DIMY - 1; j++) {
				for (i = 1; i < m_DIMX - 1; i++) {
					idxCenter = i + j * m_DIMX + k * m_DIMXY;

					if (m_imLab[idxCenter] != 0) {
						
						// Update neighbors
						for (register int ii = 0; ii < 26; ii++) {
							idxNgbh = idxCenter + m_indOff[ii];
							C = ( maxC - std::abs(m_imSrc[idxCenter] - m_imSrc[idxNgbh]) ) / maxC;
							theta = (float) ( C * m_imDist[idxCenter]);

							if ((theta - m_imDist[idxNgbh]) > 0.01) {
								m_imDist[idxNgbh] += theta;
								m_imDist[idxNgbh] /= 2;
								m_imLab[idxNgbh] = m_imLab[idxCenter];
								// Keep iterating
								converged = false;
							}
						}
					}
				}
			}
		}
		cont++;
		if (cont == maxIt) break;	
	}
	//std::cout << "Iterations " << cont << endl;
}

template<typename SrcPixelType, typename LabPixelType> 
void CleverSeg<SrcPixelType, LabPixelType>
::DoSegmentation() {
		calculateNeighboursAndInitialWeights();
		CleverSegDefault();
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::GetLabeImage(std::vector<LabPixelType>& imLab) {
    imLab.resize(m_DIMXYZ);
    std::copy(m_imLab.begin(), m_imLab.end(), imLab.begin());
}

template<typename SrcPixelType, typename LabPixelType>
void CleverSeg<SrcPixelType, LabPixelType>
::GetForegroundmage(std::vector<LabPixelType>& imFgrd) {
    long index;
    imFgrd.resize(m_DIMXYZ);
    index = 0;
    for(index = 0; index < m_DIMXYZ; index++) {
        imFgrd[index] = m_imLab[index] == 1 ? 1 :0;
    }

}
} // end CS
