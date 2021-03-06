//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVSMTKModelInformation - Light object for holding information
// about a smtk model object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSMTKModelInformation_h
#define __vtkPVSMTKModelInformation_h

#include "ModelBridgeClientModule.h"
#include "smtk/common/UUID.h"
#include "vtkPVInformation.h"
#include <map>
#include <string>

class MODELBRIDGECLIENT_EXPORT vtkPVSMTKModelInformation : public vtkPVInformation
{
public:
  static vtkPVSMTKModelInformation* New();
  vtkTypeMacro(vtkPVSMTKModelInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  void AddInformation(vtkPVInformation* info) override;

  void CopyToStream(vtkClientServerStream*) override { ; }
  void CopyFromStream(const vtkClientServerStream*) override { ; }

  // Description:
  // return the blockid given a entity UUID.
  // Caution: This will be slow if there are many blocks in the model
  virtual bool GetBlockId(const smtk::common::UUID& uuid, unsigned int& bid);
  // return the modelentityID given a blockid.
  // Caution: There is no valid check for this for performance reason
  virtual const smtk::common::UUID& GetModelEntityId(unsigned int bid);

  virtual const smtk::common::UUID& GetModelUUID();

  // Description:
  // return UUIDs to BlockId map for all blocks
  // virtual smtk::common::UUIDs GetBlockUUIDs() const;
  const std::map<smtk::common::UUID, vtkIdType>& GetUUID2BlockIdMap() const
  {
    return this->UUID2BlockIdMap;
  }

protected:
  vtkPVSMTKModelInformation();
  ~vtkPVSMTKModelInformation() override;

  std::map<smtk::common::UUID, vtkIdType> UUID2BlockIdMap;
  std::map<vtkIdType, smtk::common::UUID> BlockId2UUIDMap;
  smtk::common::UUID m_ModelUUID;

private:
  vtkPVSMTKModelInformation(const vtkPVSMTKModelInformation&); // Not implemented
  void operator=(const vtkPVSMTKModelInformation&);            // Not implemented

  //  smtk::common::UUID m_dummyID;
};

#endif
