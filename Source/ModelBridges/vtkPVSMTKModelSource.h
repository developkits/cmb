//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkPVSMTKModelSource_h
#define __vtkPVSMTKModelSource_h

#include "ModelBridgeClientModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include <map>
#include <string>

class vtkModelManagerWrapper;

/**\brief A VTK source for passing SMTK Model Manager to its super class.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with a tessellation entry.
  */
class MODELBRIDGECLIENT_EXPORT vtkPVSMTKModelSource : public vtkModelMultiBlockSource
{
public:
  static vtkPVSMTKModelSource* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkPVSMTKModelSource, vtkModelMultiBlockSource);

  // Description:
  // Set model manager wrapper
  void SetModelManagerWrapper(vtkModelManagerWrapper* modelManager);
  vtkGetObjectMacro(ModelManagerWrapper, vtkModelManagerWrapper);

  // Description:
  // Model entity ID that this source will be built upon.
  // Forwarded to vtkModelMultiBlockSource
  void SetModelEntityID(const char*) override;
  char* GetModelEntityID() override;

  // Description:
  // Forwarded to vtkModelMultiBlockSource
  virtual void SetShowAnalysisMesh(int val) { this->SetShowAnalysisTessellation(val); }

  void MarkDirty() { this->Dirty(); }
protected:
  vtkPVSMTKModelSource();
  ~vtkPVSMTKModelSource() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  // Reference model Manager wrapper:
  vtkModelManagerWrapper* ModelManagerWrapper;

private:
  vtkPVSMTKModelSource(const vtkPVSMTKModelSource&); // Not implemented.
  void operator=(const vtkPVSMTKModelSource&);       // Not implemented.
};

#endif // __vtkPVSMTKModelSource_h
