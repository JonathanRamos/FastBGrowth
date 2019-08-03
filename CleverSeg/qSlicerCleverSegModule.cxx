/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// CleverSeg Logic includes
#include <vtkSlicerCleverSegLogic.h>

// CleverSeg includes
#include "qSlicerCleverSegModule.h"
#include "qSlicerCleverSegModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerCleverSegModule, qSlicerCleverSegModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCleverSegModulePrivate
{
public:
  qSlicerCleverSegModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCleverSegModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCleverSegModulePrivate::qSlicerCleverSegModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCleverSegModule methods

//-----------------------------------------------------------------------------
qSlicerCleverSegModule::qSlicerCleverSegModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCleverSegModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCleverSegModule::~qSlicerCleverSegModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCleverSegModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerCleverSegModule::acknowledgementText() const
{
  return "This work was funded by FASPESP grant 2017/23780-2";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCleverSegModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jonathan Ramos (University of São Paulo)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCleverSegModule::icon() const
{
  return QIcon(":/Icons/CleverSeg.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCleverSegModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCleverSegModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerCleverSegModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerCleverSegModule
::createWidgetRepresentation()
{
  return new qSlicerCleverSegModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCleverSegModule::createLogic()
{
  return vtkSlicerCleverSegLogic::New();
}