/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerCleverSegFooBarWidget.h"
#include "ui_qSlicerCleverSegFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CleverSeg
class qSlicerCleverSegFooBarWidgetPrivate
  : public Ui_qSlicerCleverSegFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerCleverSegFooBarWidget);
protected:
  qSlicerCleverSegFooBarWidget* const q_ptr;

public:
  qSlicerCleverSegFooBarWidgetPrivate(
    qSlicerCleverSegFooBarWidget& object);
  virtual void setupUi(qSlicerCleverSegFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerCleverSegFooBarWidgetPrivate
::qSlicerCleverSegFooBarWidgetPrivate(
  qSlicerCleverSegFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerCleverSegFooBarWidgetPrivate
::setupUi(qSlicerCleverSegFooBarWidget* widget)
{
  this->Ui_qSlicerCleverSegFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerCleverSegFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerCleverSegFooBarWidget
::qSlicerCleverSegFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCleverSegFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerCleverSegFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerCleverSegFooBarWidget
::~qSlicerCleverSegFooBarWidget()
{
}
