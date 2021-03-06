//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBCommonMainWindow.h"
#include "ui_qtCMBMainWindow.h"

#include "pqCMBCommonMainWindowCore.h"

#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>

#include "pqApplicationCore.h"
#include "pqCMBRenderLog.h"
#include "pqCMBRubberBandHelper.h"
#include "pqCameraReaction.h"
#include "pqColorMapEditor.h"
#include "pqDataRepresentation.h"
#include "pqDisplayColorWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqPropertyLinks.h"
#include "pqRecentFilesMenu.h"
#include "pqRenderView.h"
#include "pqRenderViewSelectionReaction.h"
#include "pqScalarBarRepresentation.h"
#include "vtkMapper.h"

#include "pqEditCameraReaction.h"
#include "pqHelpReaction.h"
#include "pqInterfaceTracker.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqServerConnectReaction.h"
#include "pqServerDisconnectReaction.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqTimerLogReaction.h"
#include "pqViewMenuManager.h"
#include "pqWaitCursor.h"

#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkPythonInterpreter.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSmartPointer.h"
#include "vtkVariant.h"

#include "pqCoreUtilities.h"
#include "pqHelpReaction.h"
#include "pqPipelineRepresentation.h"
#include "qtCMBAboutDialog.h"
#include "qtCMBColorDialog.h"
#include "qtCMBHelpDialog.h"

#include "pqFileDialog.h"
#include "pqProxyWidget.h"
#include "pqSaveScreenshotReaction.h"
#include "pqTestUtility.h"
#include "pqTestingReaction.h"
#include "pqTimerLogReaction.h"

#include "pqExportReaction.h"
#include "pqLoadStateReaction.h"
#include "pqManagePluginsReaction.h"
#include "pqSaveStateReaction.h"
#include "vtkPVGeneralSettings.h"
#include "vtkPVRenderViewSettings.h"

#include "pqActiveObjects.h"
#include "pqCMBColorMapWidget.h"
#include "pqCMBLoadDataReaction.h"
#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"
#include "pqCMBRulerDialog.h"
#include "pqPluginIOBehavior.h"
#include "pqProxyWidgetDialog.h"
#include "qtCMBPanelsManager.h"
#include "vtkCommand.h"

#include "smtk/io/Logger.h"

#include "vtkPVConfig.h"

#include <vtksys/SystemTools.hxx>

class pqCMBCommonMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/)
    : RecentFilesMenu(0)
  {
    this->DockPanelViewMenu = 0;
  }

  ~vtkInternal() { delete this->DockPanelViewMenu; }

  Ui::qtCMBMainWindow UI;
  pqRecentFilesMenu* RecentFilesMenu;
  pqViewMenuManager* DockPanelViewMenu;

  pqPropertyLinks GUILinker;

  QList<pqOutputPort*> LastSelectionPorts;

  QPointer<pqServerConnectReaction> ServerConnectReaction;
  QPointer<pqServerDisconnectReaction> ServerDisconnectReaction;
  QPointer<pqHelpReaction> HelpReaction;
  QPointer<pqEditCameraReaction> EditCameraReaction;
  QPointer<pqTimerLogReaction> TimerLogReaction;
  QPointer<pqCMBLoadDataReaction> LoadDataReaction;
  QPointer<pqExportReaction> ExportReaction;
  QPointer<qtCMBPanelsManager> panelsManager;
  QPointer<pqProxyWidget> displayPanel;
  QPointer<pqCMBColorMapWidget> ColorEditor;
#ifdef __APPLE__
  bool prevNativeMenuBar;
#endif

  QPointer<QDockWidget> PVColorEditorDock;
};

static void add_python_path(std::string const& path)
{
  std::string const& full_path = vtksys::SystemTools::CollapseFullPath(path);
  vtkPythonInterpreter::PrependPythonPath(full_path.c_str());
}

pqCMBCommonMainWindow::pqCMBCommonMainWindow()
  : SelectionShortcut(NULL)
  , ResetCameraShortcut(NULL)
  , m_isExiting(false)
  , Internal(new vtkInternal(this))
{
  this->MainWindowCore = NULL;
  this->Internal->UI.setupUi(this);
  this->Internal->RecentFilesMenu =
    new pqRecentFilesMenu(*this->Internal->UI.menuRecentFiles, this);

  vtkProcessModule* proc_module = vtkProcessModule::GetProcessModule();
  std::string self_dir = proc_module->GetSelfDir();
  if (!self_dir.empty())
  {
#ifdef __APPLE__
    bool is_build_dir = vtksys::SystemTools::FileExists(self_dir + "/../../../../CMakeCache.txt");
#else
    bool is_build_dir = vtksys::SystemTools::FileExists(self_dir + "/../CMakeCache.txt");
#endif

    if (is_build_dir)
    {
#ifndef USE_SYSTEM_SMTK
      add_python_path(self_dir + "/../ThirdParty/SMTK");
#endif
#ifdef CMB_SUPERBUILD_DEVELOPER_ROOT
#ifdef _WIN32
      add_python_path(CMB_SUPERBUILD_DEVELOPER_ROOT "/bin/Lib/site-packages");
      add_python_path(CMB_SUPERBUILD_DEVELOPER_ROOT "/lib/site-packages");
#else
      add_python_path(CMB_SUPERBUILD_DEVELOPER_ROOT "/lib/python2.7/site-packages");
      add_python_path(
        CMB_SUPERBUILD_DEVELOPER_ROOT "/lib/paraview-" PARAVIEW_VERSION "/site-packages");
#endif
#endif
    }
    else
    {
#ifdef _WIN32
      add_python_path(self_dir + "/Lib/site-packages");
#elif __APPLE__
/* handled by ParaView */
#else
      // The shared forward executable lives in the lib/cmb-${cmb_version} directory.
      add_python_path(self_dir + "/../python2.7/site-packages");
#endif
    }
  }

  pqApplicationCore* core = pqApplicationCore::instance();

  QObject::connect(
    this->Internal->UI.action_Exit, SIGNAL(triggered()), this, SLOT(onAskedToExit()));

  QObject::connect(this, SIGNAL(userAcceptsExit()), core, SLOT(quit()));

  QObject::connect(this->Internal->UI.action_Help, SIGNAL(triggered()), this, SLOT(onHelpHelp()));

  QObject::connect(this->Internal->UI.action_About, SIGNAL(triggered()), this, SLOT(onHelpAbout()));

  this->Internal->ServerConnectReaction =
    new pqServerConnectReaction(this->Internal->UI.actionServerConnect);
  this->Internal->ServerDisconnectReaction =
    new pqServerDisconnectReaction(this->Internal->UI.actionServerDisconnect);

  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)), this, SLOT(onViewChanged()));

  // Setup some standard shortcuts
  this->getMainDialog()->action_Close->setShortcuts(QKeySequence::Close);
  this->getMainDialog()->action_Exit->setShortcuts(QKeySequence::Quit);
  this->getMainDialog()->action_Help->setShortcuts(QKeySequence::HelpContents);
  this->getMainDialog()->action_Open_File->setShortcuts(QKeySequence::Open);
  this->getMainDialog()->action_Save_As->setShortcuts(QKeySequence::SaveAs);
  this->getMainDialog()->action_Save_Data->setShortcuts(QKeySequence::Save);
  this->getMainDialog()->actionNew->setShortcuts(QKeySequence::New);
  this->getMainDialog()->actionRedo->setShortcuts(QKeySequence::Redo);
  this->getMainDialog()->actionUndo->setShortcuts(QKeySequence::Undo);

  // By default we want the close session action to be hidden (we might get rid of it
  // all together at some point
  this->getMainDialog()->action_Close_Session->setVisible(false);

  // Turn off the "Export Model" menu item by default (ModelBuilder will turn it on)
  this->getMainDialog()->action_Export_Model->setVisible(false);

#ifdef __APPLE__
  this->Internal->prevNativeMenuBar = this->menuBar()->isNativeMenuBar();
#endif
}

pqCMBCommonMainWindow::~pqCMBCommonMainWindow()
{
  this->m_isExiting = true;
  delete this->Internal;

  if (this->SelectionShortcut)
  {
    delete this->SelectionShortcut;
  }

  if (this->ResetCameraShortcut)
  {
    delete this->ResetCameraShortcut;
  }
}

qtCMBPanelsManager* pqCMBCommonMainWindow::panelsManager()
{
  if (!this->Internal->panelsManager)
  {
    this->Internal->panelsManager = new qtCMBPanelsManager(this);
  }
  return this->Internal->panelsManager;
}

inline void customizePanelVisibility(vtkSMProxy* smProxy, const char* propName,
  const char* panelvis, bool removePanelVisForRepresentation = false)
{
  vtkSMProperty* smProp = smProxy->GetProperty(propName);
  if (smProp)
  {
    smProp->SetPanelVisibility(panelvis);
    if (removePanelVisForRepresentation)
      smProp->SetPanelVisibilityDefaultForRepresentation(NULL);
  }
}

pqProxyWidget* pqCMBCommonMainWindow::displayPanel(vtkSMProxy* repProxy)
{
  if (this->Internal->displayPanel && this->Internal->displayPanel->proxy() != repProxy)
  {
    delete this->Internal->displayPanel;
    this->Internal->displayPanel = NULL;
  }

  if (!this->Internal->displayPanel && repProxy)
  {
    // customize Display properties
    customizePanelVisibility(repProxy, "PolarAxes", "advanced");
    customizePanelVisibility(repProxy, "UseDataPartitions", "advanced", true);

    for (size_t index = 0; index < repProxy->GetNumberOfPropertyGroups(); index++)
    {
      int group_tag = static_cast<int>(index);
      vtkSMPropertyGroup* group = repProxy->GetPropertyGroup(group_tag);
      if (!group)
        continue;
      QString grplabel = group->GetXMLLabel();
      // make Transforming related components default
      if (grplabel == "Transforming")
      {
        for (size_t j = 0; j < group->GetNumberOfProperties(); j++)
        {
          group->GetProperty(static_cast<unsigned int>(j))->SetPanelVisibility("default");
        }

        group->SetPanelVisibility("default");
      }
      // make point Gaussian related components advanced
      else if (grplabel == "Point Gaussian")
      {
        for (size_t j = 0; j < group->GetNumberOfProperties(); j++)
        {
          group->GetProperty(static_cast<unsigned int>(j))->SetPanelVisibility("advanced");
        }
        group->SetPanelVisibility("advanced");
      }
    }

    this->Internal->displayPanel = new pqProxyWidget(repProxy, this);
    this->Internal->displayPanel->setView(this->MainWindowCore->activeRenderView());
    this->Internal->displayPanel->setApplyChangesImmediately(true);

    QObject::connect(this->Internal->displayPanel, SIGNAL(changeFinished()), this->MainWindowCore,
      SLOT(requestRender()));
  }

  return this->Internal->displayPanel;
}

pqCMBColorMapWidget* pqCMBCommonMainWindow::colorEditor(QWidget* p)
{
  if (!this->Internal->ColorEditor)
  {
    this->Internal->ColorEditor = new pqCMBColorMapWidget(p);
  }
  return this->Internal->ColorEditor;
}

void pqCMBCommonMainWindow::initInspectorDock()
{
  this->Internal->UI.faceParametersDock->setEnabled(false);
}

QDockWidget* pqCMBCommonMainWindow::initPVColorEditorDock()
{
  if (!this->Internal->PVColorEditorDock)
  {
    QPointer<pqColorMapEditor> colorWidget = new pqColorMapEditor(this);
    this->Internal->PVColorEditorDock = this->panelsManager()->createDockWidget(this, colorWidget,
      qtCMBPanelsManager::type2String(qtCMBPanelsManager::COLORMAP), Qt::RightDockWidgetArea, NULL);
    this->Internal->PVColorEditorDock->hide();
    pqApplicationCore::instance()->unRegisterManager("COLOR_EDITOR_PANEL");
    pqApplicationCore::instance()->registerManager(
      "COLOR_EDITOR_PANEL", this->Internal->PVColorEditorDock);
  }
  return this->Internal->PVColorEditorDock;
}

void pqCMBCommonMainWindow::appendDatasetNameToTitle(const QString& strTitle)
{
  QString currentTitle =
    QApplication::applicationName().append(" ").append(QApplication::applicationVersion());

  QString newTitle =
    strTitle.isEmpty() ? currentTitle : currentTitle.append(" ( ").append(strTitle).append(" )");
  this->setWindowTitle(newTitle);
}

void pqCMBCommonMainWindow::onViewChanged()
{
  if (this->MainWindowCore && this->MainWindowCore->activeRenderView())
  {
    this->setCentralWidget(this->MainWindowCore->activeRenderView()->widget());
    // Allow multiple representation selection
    this->MainWindowCore->activeRenderView()->setUseMultipleRepresentationSelection(true);

    QObject::connect(this->MainWindowCore->activeRenderView(),
      SIGNAL(multipleSelected(QList<pqOutputPort*>)), this,
      SLOT(onViewSelected(QList<pqOutputPort*>)));

    this->Internal->EditCameraReaction = new pqEditCameraReaction(
      this->Internal->UI.actionAdjustCamera, this->MainWindowCore->activeRenderView());

    vtkPVRenderViewSettings::GetInstance()->SetResolveCoincidentTopology(
      vtkPVRenderViewSettings::OFFSET_FACES);
    vtkPVRenderViewSettings::GetInstance()->SetPolygonOffsetParameters(1.0, 0.5);

    // NOTE: This is a temporary HACK. After server is created (paraview settings were set),
    // CMB will change the coincident geometry settings. This only works with buildin server (same process)
    // and only for openGL2, UNTIL paraview exposes these parameters from its PVRenderViewSettings.
    vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(2.0, 2.0);
    vtkMapper::SetResolveCoincidentTopologyLineOffsetParameters(1.0, 1.0);
  }
}

void pqCMBCommonMainWindow::onLockViewSize(bool lock)
{
  if (!lock)
  {
    this->centralWidget()->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  }
  else
  {
    this->centralWidget()->setMaximumSize(QSize(300, 300));
  }
}

void pqCMBCommonMainWindow::initMainWindowCore()
{
  if (!this->MainWindowCore)
  {
    return;
  }

  // Create builtin server connection.
  //pqApplicationCore::instance()->getObjectBuilder()->blockSignals(true);
  pqServer* server =
    pqApplicationCore::instance()->getObjectBuilder()->createServer(pqServerResource("builtin:"));
  //pqApplicationCore::instance()->getObjectBuilder()->blockSignals(false);
  this->MainWindowCore->onServerCreationFinished(server);

  this->Internal->DockPanelViewMenu = new pqViewMenuManager(this, this->Internal->UI.menuShow);

  this->initInspectorDock();
  this->initPVColorEditorDock();

  QObject::connect(this->MainWindowCore, SIGNAL(enableExternalProcesses(bool)), this,
    SLOT(onEnableExternalProcesses(bool)));

  pqCMBRecentlyUsedResourceLoaderImplementatation* rli =
    new pqCMBRecentlyUsedResourceLoaderImplementatation(this->MainWindowCore);
  pqApplicationCore::instance()->interfaceTracker()->addInterface(rli);

  //  QObject::connect(this->Internal->UI.action_Open_File,
  //    SIGNAL(triggered()), this->MainWindowCore, SLOT(onFileOpen()));
  QObject::connect(this->Internal->UI.action_Save_Data, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(onSaveData()));

  QObject::connect(this->Internal->UI.action_Save_As, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(onSaveAsData()));

  QObject::connect(this->Internal->UI.action_Close, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(onCloseData()));

  //<addaction name="actionToolsDumpWidgetNames" />
  //new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Record Test...")
  //  << pqSetName("actionToolsRecordTest"),
  //  pqTestingReaction::RECORD);
  if (pqApplicationCore::instance()->testUtility())
  {
    QAction* recordAct = this->Internal->UI.menu_Tools->addAction("Record Test...");
    QObject::connect(recordAct, SIGNAL(triggered()), this, SLOT(onRecordTest()));
    QObject::connect(pqApplicationCore::instance()->testUtility()->recorder(), SIGNAL(stopped()),
      this, SLOT(onRecordTestStopped()), Qt::QueuedConnection);
    QObject::connect(pqApplicationCore::instance()->testUtility()->eventTranslator(),
      SIGNAL(stopped()), this, SLOT(onRecordTestStopped()), Qt::QueuedConnection);

    new pqTestingReaction(
      this->Internal->UI.menu_Tools->addAction("Play Test...") << pqSetName("actionToolsPlayTest"),
      pqTestingReaction::PLAYBACK, Qt::QueuedConnection);
  }

  QObject::connect(this->Internal->UI.actionLock_View_Size, SIGNAL(toggled(bool)), this,
    SLOT(onLockViewSize(bool)));
  //  new pqSaveScreenshotReaction(this->Internal->UI.actionSaveScreenshot);
  QObject::connect(this->Internal->UI.actionSaveScreenshot, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(onSaveScreenshot()));

  //  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Lock View Size")
  //    << pqSetName("actionTesting_Window_Size"),
  //    pqTestingReaction::LOCK_VIEW_SIZE);
  //  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Lock View Size Custom...")
  //    << pqSetName("actionTesting_Window_Size_Custom"),
  //    pqTestingReaction::LOCK_VIEW_SIZE_CUSTOM);
  this->Internal->UI.menu_Tools->addSeparator();
  new pqTimerLogReaction(
    this->Internal->UI.menu_Tools->addAction("Timer Log") << pqSetName("actionToolsTimerLog"));

  // Add Ruler
  this->Internal->UI.menu_Tools->addSeparator();
  QAction* actionRuler = this->Internal->UI.menu_Tools->addAction("&Ruler")
    << pqSetName("actionToolsRuler");
  QObject::connect(actionRuler, SIGNAL(triggered()), this, SLOT(createRulerDialog()));

  this->Internal->UI.menu_Tools->addSeparator();

  // Add configuration dialog for the Axes Grid
  QAction* actionAxesGridConfigurations =
    this->Internal->UI.menuAxes_Grid->addAction("Configurations")
    << pqSetName("actionAxesGridConfigurations");
  QObject::connect(actionAxesGridConfigurations, SIGNAL(triggered()), this,
    SLOT(createAxesGridConfigurationDialog()));

#ifdef BUILD_SHARED_LIBS
  // Add support for importing plugins only if ParaView was built shared.
  new pqManagePluginsReaction(this->Internal->UI.menu_Tools->addAction("Manage Plugins...")
    << pqSetName("actionManage_Plugins"));
#else
  QAction* action2 = this->Internal->UI.menu_Tools->addAction("Manage Plugins...");
  action2->setEnabled(false);
  action2->setToolTip("Use BUILD_SHARED:ON while compiling to enable plugin support.");
  action2->setStatusTip(action2->toolTip());
#endif

  QAction* ssaction = this->Internal->UI.menu_Tools->addAction("Save State (Debug)")
    << pqSetName("actionToolsSaveState");
  //  QAction* lsaction = this->Internal->UI.menu_Tools->addAction("Load State (Debug)")
  //    << pqSetName("actionToolsLoadState");
  new pqSaveStateReaction(ssaction);
  //  new pqLoadStateReaction(lsaction);
  this->Internal->UI.menu_Tools->addSeparator();
  this->Internal->UI.menu_Tools->addAction(this->Internal->UI.actionClear_Log_WIndow);
  QObject::connect(this->Internal->UI.actionSettings, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(onEditSettings()));

  QObject::connect(this->Internal->UI.actionReset_Camera, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(resetCamera()));
  QObject::connect(this->Internal->UI.actionView_Positive_X, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosX()));
  QObject::connect(this->Internal->UI.actionView_Negative_X, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegX()));
  QObject::connect(this->Internal->UI.actionView_Positive_Y, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosY()));
  QObject::connect(this->Internal->UI.actionView_Negative_Y, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegY()));
  QObject::connect(this->Internal->UI.actionView_Positive_Z, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosZ()));
  QObject::connect(this->Internal->UI.actionView_Negative_Z, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegZ()));

  // Setup Camera Interaction Connections
  QObject::connect(this->Internal->UI.action2D_camera, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(set2DCameraInteraction()));
  QObject::connect(this->Internal->UI.action3D_camera, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(set3DCameraInteraction()));
  QObject::connect(this->MainWindowCore, SIGNAL(enableCameraInteractionModeChanged(bool)), this,
    SLOT(onEnableCameraInteractionModeChanges(bool)));
  QObject::connect(this->MainWindowCore, SIGNAL(cameraInteractionModeChangedTo2D(bool)), this,
    SLOT(onCameraInteractionModeChangeTo2D(bool)));

  // we want to rotate the object, but the trick here is about camera
  new pqCameraReaction(
    this->Internal->UI.actionView_Rotate_90_cw, pqCameraReaction::ROTATE_CAMERA_CCW);
  new pqCameraReaction(
    this->Internal->UI.actionView_Rotate_90_ccw, pqCameraReaction::ROTATE_CAMERA_CW);
  // Set up Zoom to Box
  new pqRenderViewSelectionReaction(
    this->Internal->UI.actionZoomToBox, NULL, pqRenderViewSelectionReaction::ZOOM_TO_BOX);
  // Set up Zoom to Selection
  QObject::connect(this->Internal->UI.actionZoomToSelection, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(zoomToSelection()));
  //Set up Axes Grid bar
  QObject::connect(this->Internal->UI.actionAxesGrid, SIGNAL(toggled(bool)), this->MainWindowCore,
    SLOT(setShowAxisGrid(bool)));
  // Set up Center Axes toolbar.
  QObject::connect(this->Internal->UI.actionShowCenterAxes, SIGNAL(toggled(bool)),
    this->MainWindowCore, SLOT(setCenterAxesVisibility(bool)));
  QObject::connect(this->Internal->UI.actionResetCenter, SIGNAL(triggered()), this->MainWindowCore,
    SLOT(resetCenterOfRotationToCenterOfCurrentData()));
  QObject::connect(this->Internal->UI.actionPickCenter, SIGNAL(toggled(bool)), this->MainWindowCore,
    SLOT(pickCenterOfRotation(bool)));
  QObject::connect(
    this->MainWindowCore, SIGNAL(disableAxisChange()), this, SLOT(disableAxisChange()));
  QObject::connect(
    this->MainWindowCore, SIGNAL(enableAxisChange()), this, SLOT(enableAxisChange()));

  //QObject::connect(
  //  this->MainWindowCore, SIGNAL(enableShowCenterAxis(bool)),
  //  this, SLOT(onShowCenterAxisChanged(bool)), Qt::QueuedConnection);
  QObject::connect(this->MainWindowCore, SIGNAL(enableResetCenter(bool)),
    this->Internal->UI.actionResetCenter, SLOT(setEnabled(bool)));
  QObject::connect(this->MainWindowCore, SIGNAL(enablePickCenter(bool)),
    this->Internal->UI.actionPickCenter, SLOT(setEnabled(bool)));
  QObject::connect(this->MainWindowCore, SIGNAL(pickingCenter(bool)),
    this->Internal->UI.actionPickCenter, SLOT(setChecked(bool)));

  QObject::connect(this->Internal->UI.actionLinkCenterWithFocal, SIGNAL(toggled(bool)),
    this->MainWindowCore, SLOT(linkCenterWithFocalPoint(bool)));

  // setup some statusBar stuff
  // this->MainWindowCore->setupProcessBar(this->statusBar());
  this->onViewChanged();

  // Connect the render log to the update log window.
  QObject::connect(this->MainWindowCore->renderLog(), SIGNAL(renderLog(const smtk::io::Logger&)),
    this, SLOT(updateLog(const smtk::io::Logger&)));

  //setup universal shortcuts for all the suite applications
  //these all must come after we call onViewChanged!

  //setup the selection shortcut. Note it will only fire
  //when the central widget ( 3d render window ) has focus
  this->SelectionShortcut = new QShortcut(QKeySequence(tr("S")), this->centralWidget());
  this->SelectionShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(
    this->SelectionShortcut, SIGNAL(activated()), this, SLOT(onSelectionShortcutActivated()));

  //setup the 3d render window reset camera shortcut.
  //note that it will only fire when the 3d render window has focus
  this->ResetCameraShortcut = new QShortcut(Qt::Key_Space, this->centralWidget());
  this->ResetCameraShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(
    this->ResetCameraShortcut, SIGNAL(activated()), this->MainWindowCore, SLOT(resetCamera()));

  vtkPVGeneralSettings::GetInstance()->SetScalarBarMode(vtkPVGeneralSettings::MANUAL_SCALAR_BARS);

  // new plugin io behavior to add more readers from plugin
  this->Internal->LoadDataReaction = new pqCMBLoadDataReaction(this->Internal->UI.action_Open_File);

  QObject::connect(
    this->Internal->LoadDataReaction, SIGNAL(multiFileLoad()), this, SLOT(loadMultiFilesStart()));
  QObject::connect(this->Internal->LoadDataReaction, SIGNAL(doneMultiFileLoad()), this,
    SLOT(loadMultiFilesStop()));

  this->Internal->LoadDataReaction->setPluginIOBehavior(new pqPluginIOBehavior(this));
  if (this->MainWindowCore->programDirectory())
  {
    this->Internal->LoadDataReaction->setProgramDirectory(this->MainWindowCore->programDirectory());
  }

  // export scene plugins
  this->Internal->ExportReaction = new pqExportReaction(this->Internal->UI.action_Export);

  // Set up a callback to before further intialization once the application
  // event loop starts.
  QTimer::singleShot(100, this->MainWindowCore, SLOT(applicationInitialize()));

  // install eventfilter before PV's mouse binds so that CMB's setting has
  // higher priority than PV's.
  if (pqActiveObjects::instance().activeView() &&
    pqActiveObjects::instance().activeView()->getProxy()->IsA("vtkSMRenderViewProxy"))
  {
    pqActiveObjects::instance().activeView()->widget()->installEventFilter(this);
  }
}

Ui::qtCMBMainWindow* pqCMBCommonMainWindow::getMainDialog()
{
  return &this->Internal->UI;
}

QList<pqOutputPort*>& pqCMBCommonMainWindow::getLastSelectionPorts()
{
  return this->Internal->LastSelectionPorts;
}

pqCMBLoadDataReaction* pqCMBCommonMainWindow::loadDataReaction()
{
  return this->Internal->LoadDataReaction;
}

void pqCMBCommonMainWindow::addControlPanel(QWidget* panel)
{
  if (panel)
  {
    // panel->setParent(this->Internal->UI.dockWidgetContents_2);
    this->Internal->UI.dockWidgetContents_2->layout()->addWidget(panel);
  }
}

void pqCMBCommonMainWindow::updateEnableState(bool data_loaded)
{
  this->Internal->UI.action_Save_Data->setEnabled(data_loaded);
  this->Internal->UI.action_Save_As->setEnabled(data_loaded);
  this->Internal->UI.action_Export_Model->setEnabled(data_loaded);
  this->Internal->UI.action_Close->setEnabled(data_loaded);

  this->Internal->UI.faceParametersDock->setEnabled(data_loaded);
  this->Internal->UI.toolBar_Selection->setEnabled(data_loaded);
}

// THIS PROB. SHOULD BE REMOVED!
void pqCMBCommonMainWindow::onViewSelected(QList<pqOutputPort*> opports)
{
  this->Internal->LastSelectionPorts.clear();
  for (int i = 0; i < opports.count(); i++)
  {
    this->Internal->LastSelectionPorts.append(opports.value(i));
  }
}

void pqCMBCommonMainWindow::clearGUI()
{
  this->updateEnableState(false);
}

bool pqCMBCommonMainWindow::compareView(
  const QString& ReferenceImage, double Threshold, ostream& Output, const QString& TempDirectory)
{
  return this->MainWindowCore->compareView(ReferenceImage, Threshold, Output, TempDirectory);
}

void pqCMBCommonMainWindow::onEnableMenuItems(bool state)
{
  // File menu
  this->getMainDialog()->action_Open_File->setEnabled(state);
  this->getMainDialog()->action_Close->setEnabled(state);
  this->getMainDialog()->action_Save_Data->setEnabled(state);
  this->getMainDialog()->action_Save_As->setEnabled(state);
  this->getMainDialog()->action_Export_Model->setEnabled(state);
  this->getMainDialog()->action_Export->setEnabled(state);
  this->getMainDialog()->menuRecentFiles->setEnabled(state);
}

void pqCMBCommonMainWindow::initProjectManager()
{
  this->MainWindowCore->initProjectManager();
}

void pqCMBCommonMainWindow::onSelectionShortcutActivated()
{
  if (this->getMainDialog()->action_Select->isEnabled())
  {
    // trigger ignores if an action is enabled, so we only fire
    // if the application is allowing selection
    this->getMainDialog()->action_Select->trigger();
  }
}

void pqCMBCommonMainWindow::disableAxisChange()
{
  bool dis = true;
  this->Internal->UI.actionView_Positive_X->setDisabled(dis);
  this->Internal->UI.actionView_Negative_X->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Y->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Y->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Z->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Z->setDisabled(dis);
}

void pqCMBCommonMainWindow::enableAxisChange()
{
  bool dis = false;
  this->Internal->UI.actionView_Positive_X->setDisabled(dis);
  this->Internal->UI.actionView_Negative_X->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Y->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Y->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Z->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Z->setDisabled(dis);
}

void pqCMBCommonMainWindow::createRulerDialog()
{
  pqCMBRulerDialog* rulerDialog = new pqCMBRulerDialog(this);
  rulerDialog->setAttribute(Qt::WA_DeleteOnClose);
  rulerDialog->show();
}

void pqCMBCommonMainWindow::checkVisibilityAndUpdateRenderView()
{
  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());

  vtkSMProxy* AxesGridProxy = vtkSMPropertyHelper(view->getProxy(), "AxesGrid").GetAsProxy();

  int showing = vtkSMPropertyHelper(AxesGridProxy, "Visibility").GetAsInt();
  if (showing)
  {
    view->forceRender();
  }
}

void pqCMBCommonMainWindow::createAxesGridConfigurationDialog()
{
  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());

  vtkSMProxy* AxesGridProxy = vtkSMPropertyHelper(view->getProxy(), "AxesGrid").GetAsProxy();

  // let the renderview update automatically when we change value in the panel
  pqCoreUtilities::connect(
    AxesGridProxy, vtkCommand::UpdateEvent, this, SLOT(checkVisibilityAndUpdateRenderView()));

  pqProxyWidgetDialog* AxesGridDialog = new pqProxyWidgetDialog(AxesGridProxy);
  AxesGridDialog->setObjectName("pqCMBAxesGridConfigurationDialog");
  AxesGridDialog->setWindowTitle("Axes Grid Configuration");
  AxesGridDialog->setAttribute(Qt::WA_DeleteOnClose);
  AxesGridDialog->show();
}

void pqCMBCommonMainWindow::showHelpPage(const QString& url)
{
  pqHelpReaction::showHelp(url);
}

void pqCMBCommonMainWindow::onAskedToExit()
{
  // Subclasses override to check for unsaved data.
  emit userAcceptsExit();
}

void pqCMBCommonMainWindow::onRecordTest()
{
  if (pqApplicationCore::instance()->testUtility())
  {
    QString filters;
    filters += "XML Files (*.xml);;";
#ifdef QT_TESTING_WITH_PYTHON
    filters += "Python Files (*.py);;";
#endif
    filters += "All Files (*)";
    pqFileDialog fileDialog(
      NULL, pqCoreUtilities::mainWidget(), tr("Record Test"), QString(), filters);
    fileDialog.setObjectName("ToolsRecordTestDialog");
    fileDialog.setFileMode(pqFileDialog::AnyFile);
    if (fileDialog.exec() == QDialog::Accepted)
    {
#ifdef __APPLE__
      this->Internal->prevNativeMenuBar = this->menuBar()->isNativeMenuBar();
      if (this->Internal->prevNativeMenuBar)
      {
        this->menuBar()->setNativeMenuBar(false);
        this->repaint();
      }
#endif

      // set cursor to the central widget of main window.
      QCursor c = cursor();
      c.setPos(mapToGlobal(
        QPoint(this->centralWidget()->width() / 2, this->centralWidget()->height() / 2)));
      c.setShape(this->cursor().shape());
      this->centralWidget()->setCursor(c);

      pqTestingReaction::recordTest(fileDialog.getSelectedFiles()[0]);
    }
  }
}

void pqCMBCommonMainWindow::onRecordTestStopped()
{
#ifdef __APPLE__
  if (this->menuBar()->isNativeMenuBar() != this->Internal->prevNativeMenuBar)
  {
    this->menuBar()->setNativeMenuBar(this->Internal->prevNativeMenuBar);
    this->repaint();
  }
#endif
}

void pqCMBCommonMainWindow::onCameraInteractionModeChangeTo2D(bool mode)
{
  // If mode is true (2D camera) then Disable the 2D camera action and
  // Enable the 3D camera action so the user can select the 3D action
  // We also want to turn off some of the connonical views and rotations
  this->Internal->UI.action2D_camera->setEnabled(!mode);
  this->Internal->UI.action2D_camera->setChecked(mode);
  this->Internal->UI.action3D_camera->setEnabled(mode);
  this->Internal->UI.action3D_camera->setChecked(!mode);
  // Set the 6 View Direction Actions
  this->Internal->UI.actionView_Positive_X->setVisible(!mode);
  this->Internal->UI.actionView_Negative_X->setVisible(!mode);
  this->Internal->UI.actionView_Positive_Y->setVisible(!mode);
  this->Internal->UI.actionView_Negative_Y->setVisible(!mode);
  this->Internal->UI.actionView_Positive_Z->setVisible(!mode);
  // Set the Rotations
  this->Internal->UI.actionView_Rotate_90_cw->setVisible(!mode);
  this->Internal->UI.actionView_Rotate_90_ccw->setVisible(!mode);
}

void pqCMBCommonMainWindow::onEnableCameraInteractionModeChanges(bool mode)
{
  if (!mode)
  {
    this->Internal->UI.action2D_camera->setEnabled(false);
    this->Internal->UI.action3D_camera->setEnabled(false);
    return;
  }
  if (this->MainWindowCore->isUsing2DCameraInteraction())
  {
    this->Internal->UI.action3D_camera->setEnabled(true);
  }
  else
  {
    this->Internal->UI.action2D_camera->setEnabled(true);
  }
}

void pqCMBCommonMainWindow::updateLog(const smtk::io::Logger& log)
{
  QString txt(log.convertToString(false).c_str());
  this->Internal->UI.LogWidget->append(txt);
}
