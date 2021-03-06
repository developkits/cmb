//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBPointsBuilderMainWindow
// .SECTION Description
// The main window for the LIDAR application.

#ifndef __pqCMBPointsBuilderMainWindow_h
#define __pqCMBPointsBuilderMainWindow_h

#include "cmbSystemConfig.h"
#include "pqCMBCommonMainWindow.h"
#include <QTreeWidgetItem>
#include <QVariant>
#include <vtkIOStream.h>

class pqOutputPort;
class pqCMBPointsBuilderMainWindowCore;
class pqPipelineSource;
class pqCMBLIDARPieceObject;

class pqCMBPointsBuilderMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBPointsBuilderMainWindow();
  ~pqCMBPointsBuilderMainWindow() override;

public slots:

  void onDataLoaded();

  // open more files without closing loaded files
  virtual void onOpenMoreFiles();

protected slots:
  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About dialog
  void onHelpAbout() override;
  void onHelpHelp() override;

  // void onRenderRequested();
  void onViewSelected(pqOutputPort*);
  void onEnableMenuItems(bool state) override;

  void loadMultiFilesStart() override;
  void loadMultiFilesStop() override;

protected:
  using pqCMBCommonMainWindow::updateEnableState;

  void setupMenuActions();

  // Description:
  // 3D Selection from the scene methods
  void updateSelection() override;

  // Initializes the application.
  virtual void initializeApplication();

private:
  pqCMBPointsBuilderMainWindow(const pqCMBPointsBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBPointsBuilderMainWindow&);               // Not implemented.

  pqCMBPointsBuilderMainWindowCore* getThisCore();

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
