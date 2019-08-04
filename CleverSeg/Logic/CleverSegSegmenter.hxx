
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
	m_NeighborIndexOffsets.clear();
	for (k = -1; k <= 1; k++)
		for (i = -1; i <= 1; i++)
			for (j = -1; j <= 1; j++)
				if (!(i == 0 && j == 0 && k == 0))
					m_NeighborIndexOffsets.push_back( i + j * (int) m_DIMX + k * ( (int) m_DIMX * (int) m_DIMY ) );

	

	// Determine neighborhood size for computation at each voxel.
	// The neighborhood size is everywhere the same (size of m_NeighborIndexOffsets)
	// except at the edges of the volume, where the neighborhood size is 0.
	m_NumberOfNeighbors.resize(m_DIMXYZ);
	const unsigned char numberOfNeighbors = m_NeighborIndexOffsets.size();
	unsigned char* nbSizePtr = &(m_NumberOfNeighbors[0]);
	for (int z = 0; z < m_DIMZ; z++)
	{
		bool zEdge = (z == 0 || z == m_DIMZ - 1);
		for (int y = 0; y < m_DIMY; y++)
		{
			bool yEdge = (y == 0 || y == m_DIMY - 1);
			*(nbSizePtr++) = 0; // x == 0 (there is always padding, so we don'neighborNewDistance need to check if m_DimX>0)
			unsigned char nbSize = (zEdge || yEdge) ? 0 : numberOfNeighbors;
			for (int x = m_DIMX - 2; x > 0; x--)
			{
				*(nbSizePtr++) = nbSize;
			}
			*(nbSizePtr++) = 0; // x == m_DimX-1 (there is always padding, so we don'neighborNewDistance need to check if m_DimX>1)
		}
	}


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
	
    float voxDiff, strength, weightDiff = 0;
	const float theta = 0.01;
    long idxCenter, idxNgbh, i, cont = 0;
	const long maxIt = 999999;
	bool converged = false;

	while (!converged) {
		converged = true;
		for (idxCenter = 0; idxCenter < m_DIMXYZ; idxCenter++) { // for each voxel
			if (m_imLab[idxCenter] != 0) { // Dont expand unlabelled voxel
				unsigned char nbSize = m_NumberOfNeighbors[idxCenter]; // get number of neighbours
				for (unsigned int i = 0; i < nbSize; i++) { // For each one of the neighbours 
					idxNgbh = idxCenter + m_NeighborIndexOffsets[i]; // calculate neighbour index

					voxDiff = ( maxC - std::abs(m_imSrc[idxCenter] - m_imSrc[idxNgbh]) ) / maxC; // calcule shifted/normalized voxel intensity difference
					strength = (float) (voxDiff * m_imDist[idxCenter]); // calculate a new strength
					weightDiff = (float) strength - m_imDist[idxNgbh];

					if (weightDiff > theta) { // wont average if it only changes above the third/fourth decimal place
						m_imDist[idxNgbh] = (m_imDist[idxNgbh] + strength)/2; // update weight
						m_imLab[idxNgbh] = m_imLab[idxCenter]; // update label
						converged = false; // Keep iterating
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
