//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBPanelsManager -
// .SECTION Description

#ifndef __CmbColorMapWidget_h
#define __CmbColorMapWidget_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QWidget>

class vtkSMProxy;
class pqDataRepresentation;

/// pqCMBColorMapWidget is a widget that can be used to edit the active color-map,
/// if any. The panel is implemented as an auto-generated panel (similar to the
/// Properties panel) that shows the properties on the lookup-table proxy.
/// Custom widgets such as pqColorOpacityEditorWidget,
/// pqColorAnnotationsPropertyWidget, and others are used to
/// control certain properties on the proxy.
class CMBAPPCOMMON_EXPORT pqCMBColorMapWidget : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqCMBColorMapWidget(QWidget* parent = 0);
  ~pqCMBColorMapWidget() override;

public slots:
  void setDataRepresentation(pqDataRepresentation* repr);

protected slots:
  /// slot called to update the currently showing proxies.
  void updateRepresentation();

  /// slot called to update the visible widgets.
  void updatePanel();

  /// render's view when transfer function is modified.
  void renderViews();

protected:
  void setColorTransferFunction(vtkSMProxy* ctf);

private:
  Q_DISABLE_COPY(pqCMBColorMapWidget)
  class pqInternals;
  pqInternals* Internals;
};

#endif
