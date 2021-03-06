//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArc.h"
#include "vtkCMBArcAutoConnectOperator.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerAutoConnectTest1(int /*argc*/, char* /*argv*/ [])
{
  //what happens when given two outer loops that
  //share an edge

  double en[4][3] = { { 0, 0, 0 }, { -5, 5, 0 }, { 0, 10, 0 }, { 5, 5, 0 } };

  for (int i = 0; i < 3; i += 2)
  {
    vtkCMBArc* arc = vtkCMBArc::New();
    vtkCMBArc::Point pt1(en[i], i);
    vtkCMBArc::Point pt2(en[i + 1], i + 1);
    arc->SetEndNode(0, pt1);
    arc->SetEndNode(1, pt2);
  }

  vtkSmartPointer<vtkCMBArcAutoConnectOperator> connect =
    vtkSmartPointer<vtkCMBArcAutoConnectOperator>::New();

  bool valid = connect->Operate(0, 1);
  vtkIdType previousCreatedId = connect->GetCreatedArcId();
  if (!valid)
  {
    //this should pass
    return 1;
  }

  valid = connect->Operate(0, previousCreatedId);
  if (valid)
  {
    //the connected id points to an arc that is already connected on both ends
    return 1;
  }

  valid = connect->Operate(0, 1);
  if (!valid)
  {
    //this should pass
    return 1;
  }
  if (previousCreatedId == connect->GetCreatedArcId())
  {
    //this would be impossible!
    return 1;
  }

  valid = connect->Operate(0, 1);
  if (valid)
  {
    //both of these are now fully connected
    return 1;
  }

  return 0;
}
