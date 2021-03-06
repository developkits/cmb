//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSceneGenV2ContourWriter - outputs all the contours in XML format
// .SECTION Description

#ifndef __vtkSceneGenV2ContourWriter_h
#define __vtkSceneGenV2ContourWriter_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKCMBIO_EXPORT vtkSceneGenV2ContourWriter : public vtkPolyDataAlgorithm
{
public:
  static vtkSceneGenV2ContourWriter* New();
  vtkTypeMacro(vtkSceneGenV2ContourWriter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  void Write() {} //Work is done in the rquest data,  This is to make paraview happy

protected:
  vtkSceneGenV2ContourWriter();
  ~vtkSceneGenV2ContourWriter() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  char* FileName;

private:
  vtkSceneGenV2ContourWriter(const vtkSceneGenV2ContourWriter&); // Not implemented.
  void operator=(const vtkSceneGenV2ContourWriter&);             // Not implemented.
};

#endif
