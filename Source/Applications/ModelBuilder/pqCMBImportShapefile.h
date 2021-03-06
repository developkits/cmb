//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqCMBImportShapefile_h
#define __pqCMBImportShapefile_h

#include "cmbSystemConfig.h"
#include <QDialog>

class pqServer;

/// options container for pages of model builder and sim builder options
class pqCMBImportShapefile : public QDialog
{
  Q_OBJECT

public:
  pqCMBImportShapefile(pqServer* activeServer, QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~pqCMBImportShapefile() override;

  int boundaryStyle();
  int marginStyle();
  QString marginSpecification();
  QString customBoundaryFilename();

protected slots:
  virtual void chooseCustomBoundaryFile();
  virtual void setCustomBoundaryFile(const QString&);

protected:
  class pqInternal;
  pqInternal* Internal;
};

#endif // __pqCMBImportShapefile_h
