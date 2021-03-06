//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef cmbProfileFunction_h_
#define cmbProfileFunction_h_

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

#include <fstream>
#include <string>
#include <vtkBoundingBox.h>

class vtkSMSourceProxy;
class vtkPiecewiseFunction;

class CMBAPPCOMMON_EXPORT cmbProfileFunctionParameters
{
public:
  virtual ~cmbProfileFunctionParameters() {}
  virtual cmbProfileFunctionParameters* clone() = 0;
};

class CMBAPPCOMMON_EXPORT cmbProfileFunction
{
public:
  //Add here when there are more than one function
  enum FunctionType
  {
    WEDGE = 0,
    MANUAL = 1
  };
  virtual ~cmbProfileFunction() {}
  virtual FunctionType getType() const = 0;
  virtual cmbProfileFunction* clone(std::string const& name) const = 0;
  void setName(std::string const& n);
  std::string const& getName() const;
  bool write(std::ofstream& out) const;
  static cmbProfileFunction* read(std::ifstream& in, size_t count);
  virtual void setRelative(bool b) = 0;

  virtual void sendDataToProxy(
    int arc_ID, int pointID, vtkBoundingBox bbox, vtkSMSourceProxy* source) const = 0;

protected:
  virtual bool readData(std::ifstream& in, int version) = 0;
  virtual bool writeData(std::ofstream& out) const = 0;
  std::string name;
};

#endif
