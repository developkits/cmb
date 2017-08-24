//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkPythonExporter - Export info with a Python script.
// .SECTION Description
// Operator to export ModelBuilder, SimBuilder and SMTK information through
// a Python script.

#ifndef __vtkPythonExporter_h
#define __vtkPythonExporter_h

#include "ModelBridgeClientModule.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/System.h"
#include "vtkObject.h"
#include <utility> // for pair in callback method for SMTK model
#include <vector>  // for callback method for SMTK Model

class vtkModelManagerWrapper;
namespace smtk
{
namespace simulation
{
class ExportSpec;
}
}

class MODELBRIDGECLIENT_EXPORT vtkPythonExporter : public vtkObject
{
public:
  static vtkPythonExporter* New();
  vtkTypeMacro(vtkPythonExporter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // This method is for standard paraview client-server apps
  virtual void Operate(vtkModelManagerWrapper* modelMgrWrapper, const char* smtkContents);

  // This method is for *legacy* paraview client-server apps
  virtual void Operate(
    vtkModelManagerWrapper* modelMgrWrapper, const char* smtkContents, const char* exportContents);

  // This method is for standalone & test apps
  virtual void Operate(smtk::model::ManagerPtr mgr, smtk::attribute::SystemPtr simulationAttributes,
    smtk::attribute::SystemPtr exportAttributes);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Get/set the script that is to be executed.
  vtkSetStringMacro(Script);
  vtkGetStringMacro(Script);

  // Description:
  // Get/set the PYTHONPATH.
  vtkSetStringMacro(PythonPath);
  vtkGetStringMacro(PythonPath);

  // Get/set the python executable. Should only be used by non-paraview apps
  vtkSetStringMacro(PythonExecutable);
  vtkGetStringMacro(PythonExecutable);

  // Description:
  // Set model manager wrapper
  void SetModelManagerWrapper(vtkModelManagerWrapper* modelManager);
  vtkGetObjectMacro(ModelManagerWrapper, vtkModelManagerWrapper);

protected:
  vtkPythonExporter();
  ~vtkPythonExporter() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkModelManagerWrapper* modelWrapper);

  char* Script;
  char* PythonPath;
  char* PythonExecutable;
  smtk::simulation::ExportSpec* SMTKExportSpec;

private:
  vtkPythonExporter(const vtkPythonExporter&); // Not implemented.
  void operator=(const vtkPythonExporter&);    // Not implemented.

  // Reference model Manager wrapper:
  vtkModelManagerWrapper* ModelManagerWrapper;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};
#endif
