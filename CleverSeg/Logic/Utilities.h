#ifndef utilities_hxx_
#define utilities_hxx_

// std
#include <iostream>
#include <fstream>
#include <limits>

// itk
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImportImageFilter.h"
//#include "itkVTKImageToImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkRegionOfInterestImageFilter.h"
#include "vtkImageData.h"

#include <csignal>

namespace CS
{

	template<typename PixelType>
	void FindVTKImageROI(vtkImageData* im, std::vector<long>& imROI) {

		int* DIMS;
		long i, j, k, kk;
		bool foundLabel;

		DIMS = im->GetDimensions();
		foundLabel = false;
		imROI.resize(6);

		for (i = 0; i < DIMS[0]; i++)
			for (j = 0; j < DIMS[1]; j++)
				for (k = 0; k < DIMS[2]; k++) {
					if (*(static_cast<PixelType*>(im->GetScalarPointer(i, j, k))) != 0) {

						if (!foundLabel) {
							imROI[0] = i;  imROI[3] = i;
							imROI[1] = j;  imROI[4] = j;
							imROI[2] = k; imROI[5] = k;
						}
						else {
							if (i < imROI[0]) imROI[0] = i;
							if (i > imROI[3]) imROI[3] = i;
							if (j < imROI[1]) imROI[1] = j;
							if (j > imROI[4]) imROI[4] = j;
							if (k < imROI[2]) imROI[2] = k;
							if (k > imROI[5]) imROI[5] = k;
						}
						foundLabel = true;
					}
				}

		// Get Editor Radius information
		// TODO: get input from Editor
		int radius = 17;
		for (kk = 0; kk < 3; kk++) {
			if (imROI[kk] - radius >= 0) {
				imROI[kk] -= radius;
			}
			if (imROI[kk + 3] + radius < DIMS[kk] - 1) {
				imROI[kk + 3] += radius;
			}
		}
	}


	template<typename PixelType>
	void ExtractVTKImageROI(vtkImageData* im, const std::vector<long>& imROI, std::vector<PixelType>& imROIVec) {

		long i, j, k, index, DIMXYZ;

		DIMXYZ = (imROI[3] - imROI[0])*(imROI[4] - imROI[1])*(imROI[5] - imROI[2]);
		imROIVec.clear();
		imROIVec.resize(DIMXYZ);
		index = 0;
		for (k = imROI[2]; k < imROI[5]; k++)
			for (j = imROI[1]; j < imROI[4]; j++)
				for (i = imROI[0]; i < imROI[3]; i++) {
					imROIVec[index++] = *(static_cast<PixelType*>(im->GetScalarPointer(i, j, k)));
				}
	}

	template<typename PixelType>
	void UpdateVTKImageROI(const std::vector<PixelType>& imROIVec, const std::vector<long>& imROI, vtkImageData* im) {

		// Set non-ROI as zeros
		memset((PixelType*)(im->GetScalarPointer()), 0, im->GetScalarSize()*im->GetNumberOfPoints());

		PixelType* pVal;
		long i, j, k, index;
		index = 0;
		for (k = imROI[2]; k < imROI[5]; k++)
			for (j = imROI[1]; j < imROI[4]; j++)
				for (i = imROI[0]; i < imROI[3]; i++) {
					pVal = static_cast<PixelType*>(im->GetScalarPointer(i, j, k));
					*pVal = imROIVec[index++];
				}
	}

}// douher

#endif