//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBExtractCellFromDataSet -
// .SECTION Description
// extracts a single cell from a data set
// and creates a polydata with only that cell

#ifndef __vtkCMBExtractCellFromDataSet_h
#define __vtkCMBExtractCellFromDataSet_h

#include "cmbSystemConfig.h"
#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"

class VTKCMBGRAPHICS_EXPORT vtkCMBExtractCellFromDataSet : public vtkDataSetAlgorithm
{
public:
  static vtkCMBExtractCellFromDataSet* New();
  vtkTypeMacro(vtkCMBExtractCellFromDataSet, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(CellIndex, int);
  vtkGetMacro(CellIndex, int);

  //BTX
protected:
  vtkCMBExtractCellFromDataSet();
  ~vtkCMBExtractCellFromDataSet() override{};

  /// Implementation of the algorithm.
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int CellIndex;

private:
  vtkCMBExtractCellFromDataSet(const vtkCMBExtractCellFromDataSet&); // Not implemented.
  void operator=(const vtkCMBExtractCellFromDataSet&);               // Not implemented.

  int BlockIndex;
  //ETX
};

#endif
