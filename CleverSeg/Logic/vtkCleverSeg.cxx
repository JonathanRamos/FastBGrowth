#include <iostream>
#include "vtkCleverSeg.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "itkImage.h"
#include "itkTimeProbe.h"

#include "CleverSegSegmenter.h"

//vtkCxxRevisionMacro(vtkCleverSeg, "$Revision$"); //necessary?
vtkStandardNewMacro(vtkCleverSeg); //for the new() macro

//----------------------------------------------------------------------------


vtkCleverSeg::vtkCleverSeg( ) {

    SourceVol   = NULL;
    SeedVol   = NULL;
    m_fastGC = NULL;
 }


vtkCleverSeg::~vtkCleverSeg() {

    //these functions decrement reference count on the vtkImageData's (incremented by the SetMacros)
    if (this->SourceVol)
    {
      this->SetSourceVol(NULL);
    }

    if (this->SeedVol)
    {
      this->SetSeedVol(NULL);
    }

    if(m_fastGC != NULL) {
         delete m_fastGC;
    }
}

void vtkCleverSeg::Initialization() {

    InitializationFlag = false;
    if(m_fastGC == NULL) {
        m_fastGC = new CS::CleverSeg<SrcPixelType, LabPixelType>();
    }
}


void vtkCleverSeg::RunCS(){

    itk::TimeProbe timer;

    timer.Start();

    QProgressBar* computationProgressBar =  new QProgressBar;
    qSlicerApplication::application()->mainWindow()->statusBar()->addPermanentWidget(computationProgressBar);

    // Find ROI
    if(!InitializationFlag) {
        CS::FindVTKImageROI<LabPixelType>(SeedVol, m_imROI);
        std::cout << "image ROI = [" << m_imROI[0] << "," << m_imROI[1] << "," << m_imROI[2] << ";"  \
                     << m_imROI[3] << "," << m_imROI[4] << "," << m_imROI[5] << "]" << std::endl;
					 
        CS::ExtractVTKImageROI<SrcPixelType>(SourceVol, m_imROI, m_imSrcVec);
    }

    CS::ExtractVTKImageROI<LabPixelType>(SeedVol, m_imROI, m_imSeedVec);

    // Initialize CleverSeg
    std::vector<long> imSize(3);
    for(int i = 0; i < 3; i++) {
        imSize[i] = m_imROI[i + 3] - m_imROI[i];
    }

    m_fastGC->SetSourceImage(m_imSrcVec);
    m_fastGC->SetSeedlImage(m_imSeedVec);
    m_fastGC->SetImageSize(imSize);
    m_fastGC->SetWorkMode(InitializationFlag);

	cout << "Image info " << m_imSrcVec.size();
    // Do Segmentation
    m_fastGC->DoSegmentation();
    //m_fastGC->GetForegroundmage(m_imLabVec);
    m_fastGC->GetLabeImage(m_imLabVec);

    // Update result
    CS::UpdateVTKImageROI<LabPixelType>(m_imLabVec, m_imROI, SeedVol);

    delete computationProgressBar;
    timer.Stop();

    if(!InitializationFlag)
        std::cout << "Initial CleverSeg segmentation time: " << timer.GetMean() << " seconds\n";
    else
        std::cout << "Adaptive CleverSeg segmentation time: " << timer.GetMean() << " seconds\n";
	
}

void vtkCleverSeg::PrintSelf(ostream &os, vtkIndent indent){
    std::cout<<"This function has been found"<<std::endl;
}
