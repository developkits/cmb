//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVSceneGenObjectInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSceneGenObjectInformation_h
#define __vtkPVSceneGenObjectInformation_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include <string>

class vtkTransform;

class VTKCMBCLIENT_EXPORT vtkPVSceneGenObjectInformation : public vtkPVInformation
{
public:
  static vtkPVSceneGenObjectInformation* New();
  vtkTypeMacro(vtkPVSceneGenObjectInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  void AddInformation(vtkPVInformation* info) override;

  // Description:
  // Manage a serialized version of the information.
  void CopyToStream(vtkClientServerStream*) override;
  void CopyFromStream(const vtkClientServerStream*) override;

  vtkGetObjectMacro(Transform, vtkTransform);
  vtkGetVector3Macro(Translation, double);
  vtkGetVector3Macro(Orientation, double);
  vtkGetVector3Macro(Scale, double);
  vtkGetVector3Macro(Color, double);
  vtkGetMacro(NumberOfPoints, int);
  const char* GetObjectType() { return this->ObjectType.c_str(); }
  const char* GetObjectName() { return this->ObjectName.c_str(); }
  const char* GetObjectFileName() { return this->ObjectFileName.c_str(); }

  // Description:
  // Does the object have "Color" PointData?
  vtkGetMacro(HasColorPointData, bool);

  //BTX
protected:
  vtkPVSceneGenObjectInformation();
  ~vtkPVSceneGenObjectInformation() override;

  // Data information collected from remote processes.
  vtkTransform* Transform;
  double Translation[3];
  double Orientation[3];
  double Scale[3];
  double Color[3];
  int NumberOfPoints;
  bool HasColorPointData;
  std::string ObjectType;
  std::string ObjectName;
  std::string ObjectFileName;

private:
  vtkPVSceneGenObjectInformation(const vtkPVSceneGenObjectInformation&); // Not implemented
  void operator=(const vtkPVSceneGenObjectInformation&);                 // Not implemented
  //ETX
};

#endif
