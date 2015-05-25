//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBConeDialog - edits a Cone Source.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBConeDialog_h
#define __qtCMBConeDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"

class pqAutoGeneratedObjectPanel;
class QDialog;
class pqPipelineSource;
class pqRenderView;

class CMBAPPCOMMON_EXPORT qtCMBConeDialog : public QObject
{
  Q_OBJECT

public:
  qtCMBConeDialog(pqPipelineSource* coneSource,
    pqRenderView* view);
  virtual ~qtCMBConeDialog();

  int exec();

protected slots:
  void accept();
  void cancel();

protected:
  int Status;
  QDialog *MainDialog;
  pqAutoGeneratedObjectPanel* ConeSourcePanel;
};

#endif /* __qtCMBConeDialog_h */
