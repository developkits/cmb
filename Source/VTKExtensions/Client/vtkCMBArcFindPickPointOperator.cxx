//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcFindPickPointOperator.h"

#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"

vtkStandardNewMacro(vtkCMBArcFindPickPointOperator);

vtkCMBArcFindPickPointOperator::vtkCMBArcFindPickPointOperator()
  : PickedPointId(-1)
{
}

vtkCMBArcFindPickPointOperator::~vtkCMBArcFindPickPointOperator()
{
}

bool vtkCMBArcFindPickPointOperator::Operate(const vtkIdType& arcId, vtkSMOutputPort* selectionPort)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "PickPointOperator"));

  //set the arc that this operator is working on
  vtkSMPropertyHelper arcIdHelper(proxy, "ArcId");
  arcIdHelper.Set(arcId);

  bool valid = proxy->Operate(selectionPort);

  if (valid)
  {
    vtkSMPropertyHelper helper(proxy, "PickedPointId");
    helper.UpdateValueFromServer();
    this->PickedPointId = helper.GetAsIdType();

    //set this picked point to be the new selection
  }

  proxy->Delete();
  return valid;
}

void vtkCMBArcFindPickPointOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
