//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkActor.h"
#include "vtkDoubleArray.h"
#include "vtkGAMBITReader.h"
#include "vtkGAMBITWriter.h"
#include "vtkGeometryFilter.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTesting.h"
#include <math.h>
#include <string>

int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc, const_cast<const char**>(argv));
  if (!testHelper->IsFlagSpecified("-T"))
  {
    std::cerr << "Error: -T /path/to/test dir was not specified.";
    return 1;
  }

  std::string tempRoot = testHelper->GetTempDirectory();
  std::string tfilename = tempRoot + "/sphere.neu";
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleSwitch> style =
    vtkSmartPointer<vtkInteractorStyleSwitch>::New();
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle(style);

  iren->SetRenderWindow(renWin);
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->Update();

  // Lets convert the sphere into a GAMBIT surface mesh
  vtkSmartPointer<vtkGAMBITWriter> writer = vtkSmartPointer<vtkGAMBITWriter>::New();
  writer->SetFileName(tfilename.c_str());
  writer->SetInputConnection(sphere->GetOutputPort());
  writer->Write();

  // Lets read in the result
  vtkSmartPointer<vtkGAMBITReader> reader = vtkSmartPointer<vtkGAMBITReader>::New();
  reader->SetFileName(tfilename.c_str());

  vtkSmartPointer<vtkGeometryFilter> filter = vtkSmartPointer<vtkGeometryFilter>::New();
  filter->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(filter->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  renderer->AddViewProp(actor);

  iren->Initialize();
  renWin->Render();

  testHelper->SetRenderWindow(renWin);
  int retVal = testHelper->RegressionTest(75);

  if (testHelper->IsInteractiveModeSpecified())
  {
    iren->Start();
  }

  return (retVal == vtkTesting::PASSED) ? 0 : 1;
}
