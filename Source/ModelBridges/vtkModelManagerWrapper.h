//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkModelManagerWrapper_h
#define __vtkModelManagerWrapper_h

#include "ModelBridgeClientModule.h"
#include "vtkObject.h"

#include "smtk/PublicPointerDefs.h"

struct cJSON;
class vtkSMTKOperator;

// .NAME vtkModelManagerWrapper - The *really* new CMB model
// .SECTION Description
// This class exists to wrap an SMTK model into a vtkObject
// subclass whose methods can be wrapped
// in order to use ParaView's client/server framework to
// synchronize remote instances.
//
// An instance of this class is tied to a vtkSMModelManagerProxy
// on the client side. They exchange information with
// proxied calls of JSON strings.
//
// Model synchronization is accomplished by serializing the
// SMTK model into a JSON string maintained as field data on
// an instance of this class.
// Operators are also serialized (1) by this instance in order
// for the client to enumerate them and (2) by the client in
// order for this object to execute them.
//
// This model also serves as a ParaView pipeline source that
// generates multiblock data of the model for rendering.
class MODELBRIDGECLIENT_EXPORT vtkModelManagerWrapper : public vtkObject
{
public:
  static vtkModelManagerWrapper* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkModelManagerWrapper, vtkObject);

  smtk::model::ManagerPtr GetModelManager();

  vtkGetStringMacro(JSONRequest);
  vtkSetStringMacro(JSONRequest);

  void ProcessJSONRequest(vtkSMTKOperator* vsOp);
  void ProcessJSONRequest() { ProcessJSONRequest(NULL); }

  vtkGetStringMacro(JSONResponse);

  std::string CanOperatorExecute(const std::string& jsonOperator);
  std::string ApplyOperator(const std::string& jsonOperator);

  bool SetModelManagerToSource(vtkObject*);

protected:
  vtkModelManagerWrapper();
  ~vtkModelManagerWrapper() override;

  vtkSetStringMacro(JSONResponse);

  void GenerateError(cJSON* err, const std::string& errMsg, const std::string& reqId);

  char* JSONRequest;
  char* JSONResponse;
  //  char* ModelEntityID;

  // Instance model Manager:
  smtk::model::ManagerPtr ModelMgr;

private:
  vtkModelManagerWrapper(const vtkModelManagerWrapper&); // Not implemented.
  void operator=(const vtkModelManagerWrapper&);         // Not implemented.
};

#endif // __vtkModelManagerWrapper_h
