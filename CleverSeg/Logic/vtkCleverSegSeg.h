
#ifndef CleverSeg_H
#define CleverSeg_H

#include "vtkSlicerCleverSegModuleLogicExport.h"
#include "vtkImageData.h"
#include "CleverSegSegmenter.h"

#include <QProgressBar>
#include <QMainWindow>
#include <QStatusBar>
#include "qSlicerApplication.h"

const unsigned short SrcDimension = 3;
typedef float DistPixelType;											// float type pixel for cost function
typedef short SrcPixelType;
typedef unsigned char LabPixelType;

class VTK_SLICER_CLEVERSEG_MODULE_LOGIC_EXPORT vtkCleverSeg : public vtkObject
{

public:
  static vtkCleverSeg* New();
  //vtkTypeRevisionMacro(vtkCleverSegSeg,vtkObject);
  vtkTypeMacro(vtkCleverSeg,vtkObject);


  //set parameters of grow cut
  vtkSetObjectMacro(SourceVol, vtkImageData);
  vtkSetObjectMacro(SeedVol, vtkImageData);
  //vtkSetObjectMacro(OutputVol, vtkImageData);

  vtkSetMacro(InitializationFlag, bool);

  //processing functions
  void Initialization();
  void RunCS();
  void PrintSelf(ostream &os, vtkIndent indent);

protected:
  vtkCleverSeg();
  virtual ~vtkCleverSeg();

private:
  //vtk image data (from slicer)
  vtkImageData* SourceVol;
  vtkImageData* SeedVol;

  std::vector<LabPixelType> m_imSeedVec;
  std::vector<LabPixelType> m_imLabVec;
  std::vector<SrcPixelType> m_imSrcVec;
  std::vector<long> m_imROI;

  //logic code
  CS::CleverSeg<SrcPixelType, LabPixelType> *m_fastGC;

  //state variables
  bool InitializationFlag;
};
#endif
