//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBTableWidget - a CMB Tree widget.
// .SECTION Description
//  A convenience QTableWidget with extra features:
//  1.  Automatic size hints based on contents
//  2.  A check box added in a header if items have check boxes
//  3.  Navigation through columns of top level items on Tab.
//  4.  Signal emitted when user navigates beyond end of the table giving an
//      opportunity to the lister to grow the table.
//  5.  Customized Drag-n-Drop
// .SECTION Caveats

#ifndef _qtCMBTableWidget_h
#define _qtCMBTableWidget_h

#include "cmbAppCommonExport.h"
#include <QTableWidget>
#include "cmbSystemConfig.h"

class QKeyEvent;

class CMBAPPCOMMON_EXPORT qtCMBTableWidget : public QTableWidget
{
  typedef QTableWidget Superclass;
  Q_OBJECT

public:

  qtCMBTableWidget(QWidget* p = NULL);
  ~qtCMBTableWidget();

  QModelIndexList getSelectedIndexes() const
    {
    return this->selectedIndexes();
    }
public slots:

signals:
  void keyPressed(QKeyEvent*);

protected slots:
  virtual void keyPressEvent(QKeyEvent*);

};

#endif // !_qtCMBTableWidget_h
