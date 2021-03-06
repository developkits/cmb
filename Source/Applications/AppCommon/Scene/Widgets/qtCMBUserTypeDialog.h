//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBUserTypeDialog - changes the user defined type of an object.
// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBUserTypeDialog_h
#define __qtCMBUserTypeDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QObject>
class QDialog;
class pqCMBSceneNode;
namespace Ui
{
class qtObjectTypeDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBUserTypeDialog : public QObject
{
  Q_OBJECT

public:
  static void updateUserType(pqCMBSceneNode* node);

protected slots:
  void accept();
  void cancel();
  void changeObjectType();

protected:
  qtCMBUserTypeDialog(pqCMBSceneNode* node);
  ~qtCMBUserTypeDialog() override;
  void exec();

  Ui::qtObjectTypeDialog* TypeDialog;
  QDialog* MainDialog;
  pqCMBSceneNode* Node;
};

#endif /* __qtCMBUserTypeDialog_h */
