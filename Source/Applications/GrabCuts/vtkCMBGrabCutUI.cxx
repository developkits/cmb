//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBGrabCutUI.h"
#include "ui_grabCuts.h"

#include "vtkCMBGrabCutFilter.h"
#include "vtkCMBWatershedFilter.h"
#include "vtkNew.h"
#include "vtkPNGReader.h"
#include "vtkPNGWriter.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"
#include "vtkXMLPolyDataWriter.h"
#include <vtkAssemblyPath.h>
#include <vtkContourFilter.h>
#include <vtkGDALRasterReader.h>
#include <vtkImageActor.h>
#include <vtkImageBlend.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImageViewer2.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMath.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropPicker.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVariant.h>

#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include "vtkCMBImageClassFilter.h"

#include <QFileDialog>
#include <QMessageBox>

class vtkDEMImageCanvasSource2D : public vtkImageCanvasSource2D
{
public:
  static vtkDEMImageCanvasSource2D* New() { return new vtkDEMImageCanvasSource2D; }

  void SetOrigin(double* o)
  {
    Origin[0] = o[0];
    Origin[1] = o[1];
    Origin[2] = o[2];
    this->Modified();
    this->ImageData->SetOrigin(o);
  }

  void SetSpacing(double* s)
  {
    Spacing[0] = s[0];
    Spacing[1] = s[1];
    Spacing[2] = s[2];
    this->Modified();
    this->ImageData->SetSpacing(s);
  }

protected:
  int RequestInformation(vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector) override
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->WholeExtent, 6);

    outInfo->Set(vtkDataObject::SPACING(), Spacing[0], Spacing[1], Spacing[2]);
    outInfo->Set(vtkDataObject::ORIGIN(), Origin[0], Origin[1], Origin[2]);

    vtkDataObject::SetPointDataActiveScalarInfo(
      outInfo, this->ImageData->GetScalarType(), this->ImageData->GetNumberOfScalarComponents());
    return 1;
  }
  vtkDEMImageCanvasSource2D()
    : vtkImageCanvasSource2D()
  {
  }
  ~vtkDEMImageCanvasSource2D() override {}
  double Origin[3];
  double Spacing[3];
};

class vtkCMBGrabCutUI::Internal
{
public:
  Internal()
  {
    PotAlpha = 0;
    Alpha = 255;
    Radius = 3;
    UpdatePotAlpha = false;
    Forground = 255;
    Background = 0;
    PotentialBG = 55;
    PotentialFG = 200;
    shiftButtonPressed = false;
    leftMousePressed = false;

    maskActor = vtkSmartPointer<vtkImageActor>::New();
    imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    drawing = vtkSmartPointer<vtkDEMImageCanvasSource2D>::New();
    filterWaterShed = vtkSmartPointer<vtkCMBWatershedFilter>::New();
    filterGrabCuts = vtkSmartPointer<vtkCMBGrabCutFilter>::New();
    filter = filterGrabCuts;

    drawing->SetDrawColor(Forground, Forground, Forground, Alpha);

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    imageViewer->SetInputData(image);
    imageViewer->GetRenderer()->ResetCamera();
    imageViewer->GetRenderer()->SetBackground(0, 0, 0);

    imageViewer->GetRenderer()->AddActor(maskActor);

    filterWaterShed->SetInputData(0, image);
    filterWaterShed->SetInputConnection(1, drawing->GetOutputPort());
    filterWaterShed->SetForegroundValue(Forground);
    filterWaterShed->SetBackgroundValue(Background);
    filterWaterShed->SetUnlabeledValue(PotentialBG);

    filterGrabCuts->SetInputData(0, image);
    filterGrabCuts->SetInputConnection(1, drawing->GetOutputPort());
    filterGrabCuts->SetNumberOfIterations(20);
    filterGrabCuts->SetPotentialForegroundValue(PotentialFG);
    filterGrabCuts->SetPotentialBackgroundValue(PotentialBG);
    filterGrabCuts->SetForegroundValue(Forground);
    filterGrabCuts->SetBackgroundValue(Background);

    lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lineActor = vtkSmartPointer<vtkActor>::New();
    lineActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
    lineActor->SetMapper(lineMapper);

    imageViewer->GetRenderer()->AddActor(lineActor);

    maskActor->GetMapper()->SetInputConnection(drawing->GetOutputPort());

    propPicker = vtkSmartPointer<vtkPropPicker>::New();
    propPicker->PickFromListOn();
    vtkImageActor* imageActor = imageViewer->GetImageActor();
    propPicker->AddPickList(imageActor);

    imageClassFilter = vtkSmartPointer<vtkCMBImageClassFilter>::New();
    imageClassFilter->SetForegroundValue(Forground);
    imageClassFilter->SetBackgroundValue(Background);

    contFilter = vtkSmartPointer<vtkContourFilter>::New();
    contFilter->SetInputConnection(imageClassFilter->GetOutputPort());
    contFilter->SetValue(0, 127.5);
    contFilter->ComputeGradientsOn();
    contFilter->ComputeScalarsOff();

    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(0.0, 0.0, 0.001);

    transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(contFilter->GetOutputPort());
    transformFilter->SetTransform(translation);
  }

  int Alpha;
  int PotAlpha;
  int Forground;
  int Background;
  int PotentialBG;
  int PotentialFG;
  int Radius;
  vtkSmartPointer<vtkImageActor> maskActor;
  vtkSmartPointer<vtkDEMImageCanvasSource2D> drawing;
  vtkImageAlgorithm* filter;
  vtkSmartPointer<vtkCMBWatershedFilter> filterWaterShed;
  vtkSmartPointer<vtkCMBGrabCutFilter> filterGrabCuts;
  vtkSmartPointer<vtkPolyDataMapper> lineMapper;
  vtkSmartPointer<vtkActor> lineActor;
  vtkSmartPointer<vtkImageViewer2> imageViewer;
  vtkSmartPointer<vtkPropPicker> propPicker;
  vtkSmartPointer<vtkContourFilter> contFilter;
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter;
  vtkSmartPointer<vtkCMBImageClassFilter> imageClassFilter;

  bool leftMousePressed;
  bool shiftButtonPressed;
  double LastPt[2];

  bool UpdatePotAlpha;

  void updateAlphas()
  {
    double currentColor[4];
    drawing->GetDrawColor(currentColor);
    vtkImageData* image = drawing->GetOutput();
    int* dims = image->GetDimensions();
    for (int z = 0; z < dims[2]; z++)
    {
      for (int y = 0; y < dims[1]; y++)
      {
        for (int x = 0; x < dims[0]; x++)
        {
          double pclass = image->GetScalarComponentAsDouble(x, y, z, 0);
          if (pclass == this->Forground)
          {
            drawing->SetDrawColor(this->Forground, this->Forground, this->Forground, this->Alpha);
          }
          else if (pclass == this->Background)
          {
            drawing->SetDrawColor(
              this->Background, this->Background, this->Background, this->Alpha);
          }
          else if (pclass == this->PotentialFG)
          {
            drawing->SetDrawColor(
              this->PotentialFG, this->PotentialFG, this->PotentialFG, this->PotAlpha);
          }
          else if (pclass == this->PotentialBG)
          {
            drawing->SetDrawColor(
              this->PotentialBG, this->PotentialBG, this->PotentialBG, this->PotAlpha);
          }
          else
          {
            std::cout << "Unknown class " << pclass << std::endl;
            continue;
          }
          drawing->DrawPoint(x, y);
        }
      }
    }
    drawing->SetDrawColor(currentColor);
    vtkRenderWindowInteractor* interactor = imageViewer->GetRenderWindow()->GetInteractor();
    interactor->Render();
  }
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabLeftMouseReleasedCallback : public vtkCommand
{
public:
  static vtkCMBGrabLeftMouseReleasedCallback* New()
  {
    return new vtkCMBGrabLeftMouseReleasedCallback;
  }

  void SetData(vtkCMBGrabCutUI::Internal* i) { this->internal = i; }

  void Execute(vtkObject*, unsigned long vtkNotUsed(event), void*) override
  {
    vtkRenderWindowInteractor* interactor =
      internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if (!internal->leftMousePressed)
    {
      style->OnLeftButtonUp();
      return;
    }

    if (internal->shiftButtonPressed)
    {
      internal->leftMousePressed = false;
      internal->shiftButtonPressed = false;
      style->OnLeftButtonUp();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(
      interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, renderer);

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
      (int)(0.5 + (pos[1] - origin[1]) / spacing[1]), 0 };

    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1], image_coordinate[0],
      image_coordinate[1], internal->Radius);
    style->OnLeftButtonUp();
    internal->leftMousePressed = false;
  }

private:
  vtkCMBGrabCutUI::Internal* internal;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabMouseMoveCallback : public vtkCommand
{
public:
  static vtkCMBGrabMouseMoveCallback* New() { return new vtkCMBGrabMouseMoveCallback; }

  vtkCMBGrabMouseMoveCallback() { this->internal = NULL; }

  ~vtkCMBGrabMouseMoveCallback() override { this->internal = NULL; }

  void SetData(vtkCMBGrabCutUI::Internal* i) { this->internal = i; }

  void Execute(vtkObject*, unsigned long vtkNotUsed(event), void*) override
  {
    vtkRenderWindowInteractor* interactor =
      internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if (!internal->leftMousePressed || internal->shiftButtonPressed)
    {
      style->OnMouseMove();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(
      interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode* node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
      (int)(0.5 + (pos[1] - origin[1]) / spacing[1]), 0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      style->OnMouseMove();
      return;
    }

    interactor->Render();
    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1], image_coordinate[0],
      image_coordinate[1], internal->Radius);
    internal->LastPt[0] = image_coordinate[0];
    internal->LastPt[1] = image_coordinate[1];
  }

private:
  vtkCMBGrabCutUI::Internal* internal;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabLeftMousePressCallback : public vtkCommand
{
public:
  static vtkCMBGrabLeftMousePressCallback* New() { return new vtkCMBGrabLeftMousePressCallback; }

  vtkCMBGrabLeftMousePressCallback() { this->internal = NULL; }

  ~vtkCMBGrabLeftMousePressCallback() override { this->internal = NULL; }

  void SetData(vtkCMBGrabCutUI::Internal* i) { this->internal = i; }

  void Execute(vtkObject*, unsigned long vtkNotUsed(event), void*) override
  {
    internal->leftMousePressed = true;
    vtkRenderWindowInteractor* interactor =
      internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    bool shiftKey = interactor->GetShiftKey();
    if (shiftKey && !internal->shiftButtonPressed)
    {
      internal->shiftButtonPressed = true;
    }

    // Pick at the mouse location provided by the interactor
    this->internal->propPicker->Pick(
      interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode* node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      style->OnLeftButtonDown();
      return;
    }

    if (internal->shiftButtonPressed)
    {
      style->OnLeftButtonDown();
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
      (int)(0.5 + (pos[1] - origin[1]) / spacing[1]), 0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      // Pass the event further on
      style->OnLeftButtonDown();
      return;
    }

    this->internal->drawing->FillBox(image_coordinate[0] - internal->Radius,
      image_coordinate[0] + internal->Radius, image_coordinate[1] - internal->Radius,
      image_coordinate[1] + internal->Radius);
    this->internal->leftMousePressed = true;
    this->internal->LastPt[0] = image_coordinate[0];
    this->internal->LastPt[1] = image_coordinate[1];

    interactor->Render();
    style->OnLeftButtonDown();
  }

private:
  vtkCMBGrabCutUI::Internal* internal;
};

vtkCMBGrabCutUI::vtkCMBGrabCutUI()
  : internal(new vtkCMBGrabCutUI::Internal())
{
  this->ui = new Ui_grabCuts;
  this->ui->setupUi(this);
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(this->ui->Open, SIGNAL(clicked()), this, SLOT(open()));
  connect(this->ui->SaveVTP, SIGNAL(clicked()), this, SLOT(saveVTP()));
  connect(this->ui->SaveMask, SIGNAL(clicked()), this, SLOT(saveMask()));
  connect(this->ui->clear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(this->ui->Run, SIGNAL(clicked()), this, SLOT(run()));

  connect(this->ui->NumberOfIter, SIGNAL(valueChanged(int)), this, SLOT(numberOfIterations(int)));
  connect(this->ui->DrawSize, SIGNAL(valueChanged(int)), this, SLOT(pointSize(int)));

  connect(this->ui->MinLandSize, SIGNAL(textChanged(const QString&)), this,
    SLOT(setBGFilterSize(const QString&)));
  connect(this->ui->MinWaterSize, SIGNAL(textChanged(const QString&)), this,
    SLOT(setFGFilterSize(const QString&)));
  this->ui->MinLandSize->setValidator(new QDoubleValidator(0, 1e50, 7, this->ui->MinLandSize));
  this->ui->MinWaterSize->setValidator(new QDoubleValidator(0, 1e50, 7, this->ui->MinWaterSize));

  connect(this->ui->DrawMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setDrawMode(int)));
  connect(this->ui->Algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlgorithm(int)));

  connect(this->ui->LabelTrans, SIGNAL(valueChanged(int)), this, SLOT(setTransparency(int)));
  connect(this->ui->DrawPossible, SIGNAL(clicked(bool)), this, SLOT(showPossibleLabel(bool)));

  connect(this->ui->SaveLines, SIGNAL(clicked()), this, SLOT(saveLines()));
  connect(this->ui->LoadLines, SIGNAL(clicked()), this, SLOT(loadLines()));

  this->ui->SaveLines->setEnabled(false);
  this->ui->LoadLines->setEnabled(false);
  this->ui->SaveVTP->setEnabled(false);
  this->ui->SaveMask->setEnabled(false);

  this->ui->qvtkWidget->SetRenderWindow(this->internal->imageViewer->GetRenderWindow());
  this->internal->imageViewer->SetupInteractor(
    this->ui->qvtkWidget->GetRenderWindow()->GetInteractor());

  vtkSmartPointer<vtkCMBGrabLeftMousePressCallback> pressCallback =
    vtkSmartPointer<vtkCMBGrabLeftMousePressCallback>::New();
  pressCallback->SetData(internal);
  vtkSmartPointer<vtkCMBGrabMouseMoveCallback> moveCallback =
    vtkSmartPointer<vtkCMBGrabMouseMoveCallback>::New();
  moveCallback->SetData(internal);
  vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback> release =
    vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback>::New();
  release->SetData(internal);

  vtkInteractorStyleImage* imageStyle = this->internal->imageViewer->GetInteractorStyle();
  imageStyle->AddObserver(vtkCommand::MouseMoveEvent, moveCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, pressCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, release);
}

vtkCMBGrabCutUI::~vtkCMBGrabCutUI()
{
  delete internal;
  delete ui;
}

void vtkCMBGrabCutUI::slotExit()
{
  qApp->exit();
}

void vtkCMBGrabCutUI::open()
{
  QString fileName =
    QFileDialog::getOpenFileName(NULL, tr("Open Tiff File"), "", tr("Tiff file (*.tif;*.tiff)"));
  if (fileName.isEmpty())
  {
    return;
  }

  this->ui->SaveLines->setEnabled(true);
  this->ui->LoadLines->setEnabled(true);
  this->ui->SaveVTP->setEnabled(true);
  this->ui->SaveMask->setEnabled(true);

  vtkSmartPointer<vtkGDALRasterReader> gdal_reader;
  gdal_reader = vtkSmartPointer<vtkGDALRasterReader>::New();
  gdal_reader->SetFileName(fileName.toStdString().c_str());
  gdal_reader->Update();
  vtkImageData* image = gdal_reader->GetOutput();
  this->internal->imageViewer->SetInputData(image);
  internal->filterGrabCuts->SetInputData(0, image);
  internal->filterWaterShed->SetInputData(0, image);
  internal->drawing->SetNumberOfScalarComponents(4);
  internal->drawing->SetScalarTypeToUnsignedChar();
  internal->drawing->SetExtent(image->GetExtent());
  internal->drawing->SetOrigin(image->GetOrigin());
  internal->drawing->SetSpacing(image->GetSpacing());

  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  internal->drawing->SetDrawColor(
    internal->PotentialBG, internal->PotentialBG, internal->PotentialBG, this->internal->PotAlpha);
  internal->drawing->FillBox(
    image->GetExtent()[0], image->GetExtent()[1], image->GetExtent()[2], image->GetExtent()[3]);
  internal->drawing->SetDrawColor(currentColor);
  internal->imageViewer->GetRenderer()->ResetCamera();

  vtkRenderWindowInteractor* interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::saveVTP()
{
  QString fileName = QFileDialog::getSaveFileName(
    NULL, tr("Save Current Boundary File"), "", tr("VTP File (*.vtp)"));
  if (fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());
  writer->SetInputConnection(
    internal->contFilter->GetOutputPort()); //internal->filter->GetOutputPort(2));
  writer->Write();
}

void vtkCMBGrabCutUI::saveMask()
{
  QString fileName =
    QFileDialog::getSaveFileName(NULL, tr("Save Current binary mask"), "", tr("vti file (*.vti)"));
  if (fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());
  vtkImageData* tmp = internal->imageClassFilter->GetOutput();
  writer->SetInputData(tmp);
  writer->Write();
}

void vtkCMBGrabCutUI::run()
{
  if (internal->filter == internal->filterGrabCuts.GetPointer())
  {
    internal->filterGrabCuts->DoGrabCut();
  }
  internal->filter->Update();

  internal->imageClassFilter->SetInputData(internal->filter->GetOutput(0));
  internal->imageClassFilter->Update();
  internal->contFilter->Update();
  internal->transformFilter->Update();
  internal->lineMapper->SetInputData(internal->transformFilter->GetOutput());
  vtkImageData* updateMask = internal->filter->GetOutput(1);
  vtkImageData* currentMask = internal->drawing->GetOutput();

  int* dims = updateMask->GetDimensions();
  double color[4];
  internal->drawing->GetDrawColor(color);
  for (int z = 0; z < dims[2]; z++)
  {
    for (int y = 0; y < dims[1]; y++)
    {
      for (int x = 0; x < dims[0]; x++)
      {
        double tmpC[] = { updateMask->GetScalarComponentAsFloat(x, y, z, 0),
          updateMask->GetScalarComponentAsFloat(x, y, z, 0),
          updateMask->GetScalarComponentAsFloat(x, y, z, 0),
          currentMask->GetScalarComponentAsFloat(x, y, z, 3) };
        internal->drawing->SetDrawColor(tmpC);
        internal->drawing->DrawPoint(x, y);
      }
    }
  }
  internal->drawing->SetDrawColor(color);
  vtkRenderWindowInteractor* interactor =
    this->internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::clear()
{
  vtkImageData* image = this->internal->imageViewer->GetInput();
  internal->drawing->SetNumberOfScalarComponents(4);
  internal->drawing->SetScalarTypeToUnsignedChar();
  internal->drawing->SetExtent(image->GetExtent());
  internal->drawing->SetOrigin(image->GetOrigin());
  internal->drawing->SetSpacing(image->GetSpacing());
  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  internal->drawing->SetDrawColor(
    internal->PotentialBG, internal->PotentialBG, internal->PotentialBG, this->internal->PotAlpha);
  internal->drawing->FillBox(
    image->GetExtent()[0], image->GetExtent()[1], image->GetExtent()[2], image->GetExtent()[3]);
  internal->drawing->SetDrawColor(currentColor);

  internal->lineMapper->SetInputData(NULL);

  vtkRenderWindowInteractor* interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::pointSize(int i)
{
  internal->Radius = i;
}

void vtkCMBGrabCutUI::numberOfIterations(int j)
{
  internal->filterGrabCuts->SetNumberOfIterations(j);
}

void vtkCMBGrabCutUI::showPossibleLabel(bool b)
{
  internal->UpdatePotAlpha = b;
  if (internal->UpdatePotAlpha)
  {
    internal->PotAlpha = internal->Alpha;
  }
  else
  {
    internal->PotAlpha = 0;
  }
  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  if (currentColor[0] == internal->PotentialBG || currentColor[0] == internal->PotentialFG)
  {
    currentColor[3] = internal->PotAlpha;
    internal->drawing->SetDrawColor(currentColor);
  }
  internal->updateAlphas();
}

void vtkCMBGrabCutUI::setTransparency(int t)
{
  if (t != internal->Alpha)
  {
    internal->Alpha = t;
    double currentColor[4];
    internal->drawing->GetDrawColor(currentColor);
    if (internal->UpdatePotAlpha)
    {
      internal->PotAlpha = internal->Alpha;
      currentColor[3] = internal->Alpha;
    }
    else if (currentColor[0] == internal->Background || currentColor[0] == internal->Forground)
    {
      currentColor[3] = internal->Alpha;
    }
    internal->drawing->SetDrawColor(currentColor);
    internal->updateAlphas();
  }
}

void vtkCMBGrabCutUI::setFGFilterSize(const QString& f)
{

  internal->imageClassFilter->SetMinFGSize(f.toDouble());
  internal->imageClassFilter->Update();
  internal->contFilter->Update();
  internal->transformFilter->Update();
  internal->lineMapper->SetInputData(internal->transformFilter->GetOutput());
  vtkRenderWindowInteractor* interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::setBGFilterSize(const QString& b)
{
  internal->imageClassFilter->SetMinBGSize(b.toDouble());
  internal->imageClassFilter->Update();
  internal->contFilter->Update();
  internal->transformFilter->Update();
  internal->lineMapper->SetInputData(internal->transformFilter->GetOutput());
  vtkRenderWindowInteractor* interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::setDrawMode(int m)
{
  switch (m)
  {
    case 1:
      internal->drawing->SetDrawColor(
        internal->Background, internal->Background, internal->Background, internal->Alpha);
      break;
    case 0:
      internal->drawing->SetDrawColor(
        internal->Forground, internal->Forground, internal->Forground, internal->Alpha);
      break;
    case 2:
      internal->drawing->SetDrawColor(
        internal->PotentialBG, internal->PotentialBG, internal->PotentialBG, internal->PotAlpha);
      break;
  }
}

void vtkCMBGrabCutUI::setAlgorithm(int a)
{
  switch (a)
  {
    case 0:
      internal->filter = internal->filterGrabCuts;
      this->ui->NumberOfIter->setEnabled(true);
      break;
    case 1:
      internal->filter = internal->filterWaterShed;
      this->ui->NumberOfIter->setEnabled(false);
      break;
  }
}

void vtkCMBGrabCutUI::saveLines()
{
  QString fileName =
    QFileDialog::getSaveFileName(NULL, tr("Save Current Lines"), "", tr("VTI File (*.vti)"));
  if (fileName.isEmpty())
  {
    return;
  }
  vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());

  writer->SetInputData(internal->drawing->GetOutput());
  writer->Write();
}

void vtkCMBGrabCutUI::loadLines()
{
  QString fileName =
    QFileDialog::getOpenFileName(NULL, tr("Open VTI File"), "", tr("vti file (*.vti)"));
  if (fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName(fileName.toStdString().c_str());
  reader->Update();
  vtkImageData* updateMask = reader->GetOutput();
  vtkImageData* current = internal->drawing->GetOutput();

  int* dims = updateMask->GetDimensions();
  int* cdims = current->GetDimensions();
  if (dims[2] != cdims[2] || dims[1] != cdims[1] || dims[0] != cdims[0])
  {
    QMessageBox::critical(
      this->parentWidget(), "Line file error", "Not the same size of the loaded image");
    return;
  }
  double color[4];
  internal->drawing->GetDrawColor(color);
  for (int z = 0; z < dims[2]; z++)
  {
    for (int y = 0; y < dims[1]; y++)
    {
      for (int x = 0; x < dims[0]; x++)
      {
        double tmpC[] = { updateMask->GetScalarComponentAsFloat(x, y, z, 0),
          updateMask->GetScalarComponentAsFloat(x, y, z, 0),
          updateMask->GetScalarComponentAsFloat(x, y, z, 0), 0 };
        internal->drawing->SetDrawColor(tmpC);
        internal->drawing->DrawPoint(x, y);
      }
    }
  }
  internal->drawing->SetDrawColor(color);
  internal->updateAlphas();
}
