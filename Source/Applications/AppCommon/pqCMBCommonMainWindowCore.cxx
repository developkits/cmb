/*=========================================================================

   Program: ConceptualModelBuilder
   Module:    pqCMBCommonMainWindowCore.cxx

   Copyright (c) Kitware Inc.
   All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqCMBCommonMainWindowCore.h"
#include "pqCMBAppCommonConfig.h" // for CMB_TEST_DATA_ROOT

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <QTextEdit>
//#include <QList>
#include <QDir>
#include <QDoubleSpinBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>
#include <QTemporaryFile>

#include "pqActionGroupInterface.h"
#include "pqActiveServer.h"
#include "pqApplicationCore.h"
#include "qtCMBApplicationOptionsDialog.h"
#include "pqCameraDialog.h"
#include "pqCoreUtilities.h"
#include "pqCustomFilterDefinitionModel.h"
#include "pqCustomFilterDefinitionWizard.h"
#include "pqCustomFilterManager.h"
#include "pqCustomFilterManagerModel.h"
#include "pqDataInformationWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqDockWindowInterface.h"
#include <pqFileDialog.h>
#include "pqFileDialogModel.h"
#include "pqLinksManager.h"
#include "pqObjectBuilder.h"
#include "pqObjectPanel.h"
#include "pqOptions.h"
#include "pqOutputWindow.h"
#include "pqOutputPort.h"
#include "pqParaViewBehaviors.h"
#include "pqPipelineFilter.h"
#include "pqPipelineModel.h"
#include "pqDataRepresentation.h"
#include "pqPipelineContextMenuBehavior.h"
#include "pqPluginManager.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqCMBRubberBandHelper.h"

#include "pqSelectionManager.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqServerConnectReaction.h"
#include "pqServerManagerModel.h"
#include "pqServerManagerObserver.h"
#include "pqSettings.h"
#include "pqSMAdaptor.h"
#include "pqTimerLogDisplay.h"
#include "pqToolTipTrapper.h"
#include "pqUndoStackBuilder.h"
#include "pqView.h"
#include "pqViewContextMenuManager.h"
#include "pqSaveSnapshotDialog.h"
#include "pqActiveObjects.h"
#include "pqPVApplicationCore.h"
#include "pqCMBTreeWidgetEventTranslator.h"
#include "pqEventTranslator.h"
#include "vtkPVPlugin.h"

#include <pqCoreTestUtility.h>
#include <pqUndoStack.h>
#include <pqWriterDialog.h>
#include <pqWaitCursor.h>
#include "pqContourWidget.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMSceneContourSourceProxy.h"
#include "pqCMBTabWidgetTranslator.h"

#include <QVTKWidget.h>

#include "vtkEvent.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include <vtkProcessModule.h>
#include <vtkPVDisplayInformation.h>
#include "vtkPVGenericRenderWindowInteractor.h"
#include <vtkPVOptions.h>
#include <vtkPVXMLElement.h>
#include <vtkPVXMLParser.h>
#include <vtkSmartPointer.h>
#include "vtkSMDataSourceProxy.h"
#include <vtkSMDoubleRangeDomain.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMPropertyHelper.h"
#include "vtkSMPropertyLink.h"
#include <vtkSMProxyIterator.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMProxySelectionModel.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMOutputPort.h>
#include <vtkSMStringVectorProperty.h>
#include "vtkSMRepresentationProxy.h"
#include "vtkStringArray.h"
#include <vtkToolkits.h>
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkPVContourRepresentationInfo.h"
#include "vtkPVRenderView.h"
#include "vtkDoubleArray.h"

#include "assert.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <vtksys/Process.h>
#include <vtksys/SystemTools.hxx>

#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBPreviewDialog.h"
#include "pqCMBProcessWidget.h"
#include "qtCMBProgressWidget.h"
#include "qtCMBContextMenuBehavior.h"
#include "qtCMBMeshingClient.h"
#include "QVTKWidget.h"
#include "qtCMBApplicationOptions.h"
#include "pqImageUtil.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef CMB_ENABLE_PYTHON
#include "vtkCMBInitializePython.h"
#endif

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>

//we only init the cmb plugin here as it is common across all applications
//the CMBModel_Plugin and SimbBuilderMesh_Plugin are inited in the Core of
//each application that depends on the,
PV_PLUGIN_IMPORT_INIT(CMB_Plugin)

///////////////////////////////////////////////////////////////////////////
// pqCMBCommonMainWindowCore::vtkInternal

/// Private implementation details for pqCMBCommonMainWindowCore
class pqCMBCommonMainWindowCore::vtkInternal
{
public:
  vtkInternal(QWidget* parent) :
    Parent(parent),
//    MultiViewManager(parent),
//    LookupTableManager(new pqPQLookupTableManager(parent)),
    ViewContextMenu(0),
    VariableToolbar(0),
    ToolTipTrapper(0),
    InCreateSource(false),
    LinksManager(0),
    TimerLog(0),
    StatusBar(0),
    ProjectManager(NULL),
    MeshingClient(NULL)
  {
  //this->MultiViewManager.setObjectName("MultiViewManager");
  this->CameraDialog = 0;

  this->PreviousCameraManipulationMode = this->CameraManipulationMode =
    pqCMBCommonMainWindowCore::vtkInternal::ThreeD; // default to 3D
//  this->isComparingScreenImage = false;

  }

  enum ManipulationModes {ThreeD, TwoD};

  ~vtkInternal()
  {
    delete this->ToolTipTrapper;
//    delete this->LookupTableManager;
    if (this->PreviewDialog)
      {
      delete this->PreviewDialog;
      }
    if (this->MeshingClient)
      {
      delete this->MeshingClient;
      }
    if ( this->ProjectManager)
      {
      delete this->ProjectManager;
      }

  }

  QWidget* const Parent;
  pqViewContextMenuManager *ViewContextMenu;
  pqCMBRubberBandHelper RenderViewSelectionHelper;
  QPointer<pqUndoStack> UndoStack;
  QTimer *ProcessTimer;
  pqCMBProcessWidget* ProcessBar;
  QString ProcessExecDirectory;

  QToolBar* VariableToolbar;
  QPointer<QComboBox> CameraManipulationModeBox;
  int CameraManipulationMode;
  int PreviousCameraManipulationMode;

  pqToolTipTrapper* ToolTipTrapper;

  QPointer<pqCameraDialog> CameraDialog;

  bool InCreateSource;

  // FIXME SEB QPointer<pqProxyTabWidget> ProxyPanel;
  QPointer<pqLinksManager> LinksManager;
  QPointer<pqTimerLogDisplay> TimerLog;

  QPointer<qtCMBApplicationOptionsDialog> AppSettingsDialog;
  QPointer<qtCMBApplicationOptions> CmbAppOptions;

  QPointer<pqServer> ActiveServer;

  QPointer<pqRenderView> RenderView;
  QPointer<pqCMBDisplayProxyEditor> AppearanceEditor;
  QWidget* AppearanceEditorContainer;
  QWidget* PropertyEditorContainer;

  QPointer<pqCMBPreviewDialog> PreviewDialog;

  // members to support display mouse position
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  QLabel *Position1Message;
  QLabel *Position2Message;
  QStatusBar *StatusBar;
  QLabel *StatusMessage;
  QString MessageBeforeEnterRenderView;
//  bool isComparingScreenImage;

  qtCMBProjectServerManager* ProjectManager;
  qtCMBMeshingClient* MeshingClient;
  bool CenterFocalLinked;

  // Set to true in InitializePythonEnvironment() if Finalize() should cleanup
  // Python.
  bool FinalizePython;

};

///////////////////////////////////////////////////////////////////////////
// pqCMBCommonMainWindowCore

pqCMBCommonMainWindowCore::pqCMBCommonMainWindowCore(QWidget* parent_widget) :
  ProgramKey(qtCMBProjectServerManager::NUM_PROGRAMS),
  Internal(new vtkInternal(parent_widget))
{
  this->buildDefaultBehaviors(parent_widget);

  this->setObjectName("pqCMBCommonMainWindowCore");
  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();

  this->connect(core->getObjectBuilder(),
    SIGNAL(finishedAddingServer(pqServer*)),
    this, SLOT(onServerCreationFinished(pqServer*)));

  this->connect(core->getServerManagerModel(),
    SIGNAL(aboutToRemoveServer(pqServer*)),
    this, SLOT(onRemovingServer(pqServer*)));

  // Set up the context menu manager.
  this->getViewContextMenuManager();


  // Listen to the active render module changed signals.
  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
    this, SLOT(onActiveViewChanged(pqView*)));
/*
  QObject::connect(
    &pqActiveView::instance(), SIGNAL(changed(pqView*)),
    this->selectionManager(), SLOT(setActiveView(pqView*)));


  QObject::connect(
    &pqActiveView::instance(), SIGNAL(changed(pqView*)),
    &this->Internal->RenderViewSelectionHelper, SLOT(setView(pqView*)));

  this->Internal->RenderViewSelectionHelper.setView(this->Internal->RenderView);
*/
  this->connect(builder,
    SIGNAL(readerCreated(pqPipelineSource*, const QString&)),
    this, SLOT(onReaderCreated(pqPipelineSource*, const QString&)));

  this->connect(builder, SIGNAL(viewCreated(pqView*)),
    this, SLOT(onViewCreated(pqView*)));

  QObject::connect(&this->Internal->RenderViewSelectionHelper,
                   SIGNAL(intersectionFinished(double, double, double)),
                   this,
                   SLOT(pickCenterOfRotationFinished(double, double, double)));

  QObject::connect(&this->Internal->RenderViewSelectionHelper,
    SIGNAL(enablePick(bool)),
    this, SIGNAL(enablePickCenter(bool)));
  //QObject::connect(&this->Internal->RenderViewSelectionHelper,
  //  SIGNAL(selecting(bool)),
  //  this, SIGNAL(pickingCenter(bool)));

  this->Internal->ProcessTimer = new QTimer;
  connect(this->Internal->ProcessTimer, SIGNAL(timeout()),
    this, SLOT(checkProcess()));

  this->Internal->PreviewDialog = new pqCMBPreviewDialog(NULL);
  QObject::connect(this->Internal->PreviewDialog, SIGNAL(accepted()), this, SLOT(onPreviewAccepted()));
  QObject::connect(this->Internal->PreviewDialog, SIGNAL(rejected()), this, SLOT(onPreviewRejected()));

  this->Internal->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Internal->CenterFocalLinked = false;
  this->Internal->FinalizePython = false;

  //setup the override for the tab widget testing translator
  core->testUtility()->eventTranslator()->addWidgetEventTranslator(
                    new pqCMBTabWidgetTranslator( core->testUtility() ) );

}

//-----------------------------------------------------------------------------
pqCMBCommonMainWindowCore::~pqCMBCommonMainWindowCore()
{
  pqActiveObjects::instance().setActiveView(NULL);
  delete Internal;
}

//-----------------------------------------------------------------------------
QWidget* pqCMBCommonMainWindowCore::parentWidget() const
{
  return this->Internal->Parent;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::buildDefaultBehaviors(QObject *parent_widget)
  {
  pqParaViewBehaviors *defaultBehaviors = new pqParaViewBehaviors(static_cast<QMainWindow *>(parent_widget), parent_widget);

  //we have no control over changing which default behaviors paraview is going to add.
  //for the right click context menu we are going to remove the default, since it breaks
  //cmb suite and each application will replace it with their own default context menu behavior

  //find all pqPipelineContextMenuBehavior
  QList<pqPipelineContextMenuBehavior*> contextMenuBehavior = defaultBehaviors->findChildren<pqPipelineContextMenuBehavior*>();

  //delete all the pqPipelineContextMenuBehavior so we can make our own
  foreach(pqPipelineContextMenuBehavior *b, contextMenuBehavior)
    {
    delete b;
    }
  }

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::buildRenderWindowContextMenuBehavior(QObject *parent_widget)
{
  new qtCMBContextMenuBehavior(parent_widget);
}

//----------------------------------------------------------------------------
 void pqCMBCommonMainWindowCore::loadProgramFile()
{
  const char* fileToLoad = getenv("CMB_FILE_TO_LOAD");
  if (fileToLoad)
    {
    this->onFileOpen(QStringList(QString(fileToLoad)));
    }
}

 //----------------------------------------------------------------------------
 void pqCMBCommonMainWindowCore::initProjectManager()
{
  const char* projectFile = getenv("CMB_PROJECT_FILE_PATH");
  if (projectFile)
    {
    this->Internal->ProjectManager = new qtCMBProjectServerManager(projectFile);
    //hack to make sure the qt program is loaded
    QTimer::singleShot(3000,this,SLOT(loadProgramFile()));
    }
}

//-----------------------------------------------------------------------------
pqSelectionManager* pqCMBCommonMainWindowCore::selectionManager()
{
  return pqPVApplicationCore::instance()->selectionManager();
}

//-----------------------------------------------------------------------------
pqCMBRubberBandHelper* pqCMBCommonMainWindowCore::renderViewSelectionHelper() const
{
  return &this->Internal->RenderViewSelectionHelper;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setupCameraManipulationModeBox(QToolBar* toolbar)
{
  this->Internal->CameraManipulationModeBox = new QComboBox(toolbar);
  this->Internal->CameraManipulationModeBox->setObjectName("cameraManipulationModeBox");
  toolbar->addSeparator();
  toolbar->addWidget(this->Internal->CameraManipulationModeBox);
  QStringList list;
  list << "3D Manipulation" << "2D Manipulation";
  this->Internal->CameraManipulationModeBox->addItems(list);
  this->Internal->CameraManipulationModeBox->setToolTip("Camera Manipulation Mode");
  QObject::connect(
    this->Internal->CameraManipulationModeBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(updateCameraManipulationMode(int)));
  this->Internal->CameraManipulationModeBox->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------
pqViewContextMenuManager* pqCMBCommonMainWindowCore::getViewContextMenuManager()
{
  if(!this->Internal->ViewContextMenu)
    {
    this->Internal->ViewContextMenu = new pqViewContextMenuManager(this);
    pqServerManagerModel* smModel =
      pqApplicationCore::instance()->getServerManagerModel();
    QObject::connect(smModel, SIGNAL(viewAdded(pqView*)),
      this->Internal->ViewContextMenu, SLOT(setupContextMenu(pqView*)));
    QObject::connect(smModel, SIGNAL(viewRemoved(pqView*)),
      this->Internal->ViewContextMenu, SLOT(cleanupContextMenu(pqView*)));
    }

  return this->Internal->ViewContextMenu;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setupProgressBar(QStatusBar* toolbar)
{
  qtCMBProgressWidget* const progress_bar = new qtCMBProgressWidget(toolbar);
  toolbar->addPermanentWidget(progress_bar);

  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();

  QObject::connect(progress_manager, SIGNAL(enableProgress(bool)),
                   progress_bar,     SLOT(enableProgress(bool)));

  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
                   progress_bar,     SLOT(setProgress(const QString&, int)));

  QObject::connect(progress_manager, SIGNAL(enableAbort(bool)),
                   progress_bar,      SLOT(enableAbort(bool)));

  QObject::connect(progress_bar,     SIGNAL(abortPressed()),
                   progress_manager, SLOT(triggerAbort()));

  progress_manager->addNonBlockableObject(progress_bar);
  progress_manager->addNonBlockableObject(progress_bar->getAbortButton());
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setupAppearanceEditor (QWidget *parent)
{
  QVBoxLayout* vboxlayout = new QVBoxLayout(parent);
  vboxlayout->setMargin(0);
  QWidget* container = new QWidget();
  container->setObjectName("scrollWidget");
  container->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(parent);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(container);
  vboxlayout->addWidget(s);

  (new QVBoxLayout(container))->setMargin(0);
  this->Internal->AppearanceEditorContainer = container;

}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setAppearanceEditor (pqCMBDisplayProxyEditor *displayEditor)
{
  if(this->Internal->AppearanceEditor == displayEditor)
    {
    return;
    }
  if(this->Internal->AppearanceEditor)
    {
    delete this->Internal->AppearanceEditor;
    }
  this->Internal->AppearanceEditor = displayEditor;
}

//-----------------------------------------------------------------------------
pqCMBDisplayProxyEditor* pqCMBCommonMainWindowCore::getAppearanceEditor()
{
  return this->Internal->AppearanceEditor;
}

//-----------------------------------------------------------------------------
QWidget* pqCMBCommonMainWindowCore::getAppearanceEditorContainer()
{
  return this->Internal->AppearanceEditorContainer;
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::compareView(
  const QString& referenceImage,
  double threshold,
  ostream& /*output*/,
  const QString& tempDirectory)
{
  // All tests need a 300x300 render window size.
  return pqCoreTestUtility::CompareView(
       this->activeRenderView(), referenceImage, threshold,
       tempDirectory, QSize(300, 300));
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::initializeStates()
{
  emit this->enableVariableToolbar(false);
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::makeServerConnectionIfNoneExists()
{
  if (this->getActiveServer())
    {
    return true;
    }

  pqApplicationCore* core = pqApplicationCore::instance();
  if (core->getServerManagerModel()->getNumberOfItems<pqServer*>() != 0)
    {
    // cannot really happen, however, if no active server, yet
    // server connection exists, we don't try to make a new server connection.
    return false;
    }

  return this->makeServerConnection();
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::makeServerConnection()
{
  pqServerConnectReaction::connectToServer();
  return (this->getActiveServer() != NULL);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::makeDefaultConnectionIfNoneExists()
{
  if (this->getActiveServer())
    {
    return ;
    }

  pqApplicationCore* core = pqApplicationCore::instance();
  if (core->getServerManagerModel()->getNumberOfItems<pqServer*>() != 0)
    {
    // cannot really happen, however, if no active server, yet
    // server connection exists, we don't try to make a new server connection.
    return ;
    }

  pqServerResource resource = pqServerResource("builtin:");
  core->getObjectBuilder()->createServer(resource);
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::checkForPreviewDialog()
{
  if (this->Internal->PreviewDialog->isVisible())
    {
    QMessageBox::warning(this->Internal->Parent,
      "Preview Dialog is Open!",
      "Must close Preview Dialog before the current dataset can be closed.");
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
const char* pqCMBCommonMainWindowCore::programDirectory()
{
  //if we have a project server manager and
  //we have been asked to load a file we overload
  //the default directory for the pqFileDialog
  //no way for the program running this not to have a directory
  return this->Internal->ProjectManager ?
     this->Internal->ProjectManager->programDirectory(
        this->getProgramKey()).toStdString().c_str() : NULL;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setupProcessBar(QStatusBar* toolbar)
{
  this->Internal->ProcessBar = new pqCMBProcessWidget(toolbar);
  toolbar->addPermanentWidget(this->Internal->ProcessBar, 1);

  QObject::connect(this->Internal->ProcessBar, SIGNAL(abortPressed()),
    this, SLOT(killProcess()));
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setupMousePositionDisplay(QStatusBar* toolbar)
{
  QWidget *mousePositionWidget = new QWidget;
  QGridLayout *gridLayout = new QGridLayout(mousePositionWidget);
  gridLayout->setSpacing(8);
  gridLayout->setMargin(0);
  gridLayout->setObjectName("gridLayoutForMousePosition");

  this->Internal->Position1Message = new QLabel;
  this->Internal->Position1Message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  //this->Internal->Position1Message->setFixedWidth(85);
  this->Internal->Position2Message = new QLabel;
  this->Internal->Position2Message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  //this->Internal->Position2Message->setFixedWidth(85);

  gridLayout->addWidget(this->Internal->Position1Message, 0, 0);
  gridLayout->addWidget(this->Internal->Position2Message, 0, 1);
  QSpacerItem* hSpacer = new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum);
  gridLayout->addItem(hSpacer, 0, 2);
  toolbar->insertPermanentWidget(0, mousePositionWidget, 1);

  // Also add the status message widget
  this->Internal->StatusMessage = new QLabel;
  QWidget *statusMessageWidget = new QWidget;
  QHBoxLayout *hbLayout = new QHBoxLayout(statusMessageWidget);
  hbLayout->addWidget(this->Internal->StatusMessage);
  toolbar->insertPermanentWidget(0, statusMessageWidget);

  this->Internal->StatusBar = toolbar;
}

//-----------------------------------------------------------------------------
QString&  pqCMBCommonMainWindowCore::getProcessExecDirectory() const
{
  return this->Internal->ProcessExecDirectory;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setProcessExecDirectory(QString execPath)
{
  this->Internal->ProcessExecDirectory = execPath;
}

//-----------------------------------------------------------------------------
pqCMBPreviewDialog* pqCMBCommonMainWindowCore::previewDialog()
{
  return this->Internal->PreviewDialog;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::launchLocalRemusServer()
{
  if (!this->Internal->MeshingClient)
    {
    //create the local meshserver, we keep a handle to it, so we can delete
    //it if the client throws an exception trying to connect to it
    qtCMBMeshingClient::LocalMeshServer meshserver =
                            qtCMBMeshingClient::launchLocalMeshServer();
    try
      {
      this->Internal->MeshingClient = new qtCMBMeshingClient(meshserver);
      }
    catch(...)
      {
      meshserver.LocalServerProxy->Delete();
      if(this->Internal->MeshingClient)
        {
        delete this->Internal->MeshingClient;
        this->Internal->MeshingClient = NULL;
        }

      this->Internal->ProcessBar->setMessage("Error: Failed to create Remus Server.");
      return;
      }
    }

}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::submitRemusSurfaceJob(const QString& command)
{
  this->launchLocalRemusServer();

  remus::meshtypes::SceneFile scene;
  remus::meshtypes::Mesh3DSurface surface;
  remus::common::MeshIOType mtype(scene,surface);

  //the hardcoded separator that command uses
  const QString jobData = this->Internal->ProcessExecDirectory + ";" + command;
  remus::proto::Job j =
                this->Internal->MeshingClient->submitJob(jobData.toStdString(),
                                                         mtype);
  this->monitorRemusJob(j);
}

//-----------------------------------------------------------------------------
QString pqCMBCommonMainWindowCore::submitRemusVolumeJob(
                                    const FileBasedMeshingParameters& params)
{
  if(!params.valid)
    {
    return QString();
    }

  //first launch the remus server if it hasn't been launched yet
  this->launchLocalRemusServer();

  //set the process execution directory for this job, this works since
  //we only have a single remus job running at once. This is really only
  //around because I don't have the time to refactor Scene and Geology builder
  //to be able to handle less state info being embedded in MainWindowCore
  this->setProcessExecDirectory(params.processExecutionDir);

  //construct the expected output file name from meshing
  //this->Internal->MesherOutputFileName
  const QFileInfo geometryFInfo(params.geometryFileName);
  const QString outputFileName = params.processExecutionDir + "/" +
                                 geometryFInfo.baseName() + ".3dm";

  const QString execPath = params.mesherExecPath;
  const QString inputString = QFileInfo(params.inputFilePath).fileName().toAscii().constData();

  //use ; as the hard-coded separator between args
  const QString commandStr = params.processExecutionDir + ";" + execPath
                              + ";" + inputString;

  //setup the remus submission types
  remus::meshtypes::SceneFile scene;
  remus::meshtypes::Mesh3D mesh;
  remus::common::MeshIOType mtype(scene,mesh);

  remus::proto::Job j = this->Internal->MeshingClient->submitJob(
                                                      commandStr.toStdString(),
                                                      mtype);
  this->monitorRemusJob(j);

  return outputFileName;
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::monitorRemusJob(const remus::proto::Job& j)
{
  const bool valid = this->Internal->MeshingClient->monitorJob(j);

  if(valid)
    {
    emit enableExternalProcesses(false);
    this->Internal->ProcessBar->setMessage("Process started");
    this->Internal->ProcessTimer->start(500); // check 2 a second
    }
  else
    {
    this->Internal->ProcessBar->setMessage("Process failed to launch");
    }

  this->Internal->ProcessBar->enableAbort(valid);
  return valid;
}

//----------------------------------------------------------------------------
QString pqCMBCommonMainWindowCore::remusServerEndpoint() const
{
  if(this->Internal->MeshingClient)
    {
    return QString::fromStdString(this->Internal->MeshingClient->endpoint());
    }
  return QString();
}
//----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::checkProcess()
{
  if (!this->Internal->MeshingClient)
    {
    return;
    }

  bool newStatus=false;
  remus::proto::JobStatus status =
                    this->Internal->MeshingClient->jobProgress(newStatus);


  //handle a finished or failed job first, the MeshingClient queries the status
  //right after the job is submitted. This stops us from treating a job
  //that always reports failed, as an ongoing job that never finishes
  const bool jobFinished = status.finished();
  const bool jobFailed = status.failed();
  if(jobFinished || jobFailed)
    {
    this->Internal->ProcessTimer->stop();
    this->Internal->ProcessBar->enableAbort(false);
    if(jobFinished)
      {
      emit remusCompletedNormally(this->Internal->MeshingClient->jobResults());
      }
    else if(jobFailed)
      {
      this->Internal->ProcessBar->appendToOutput("Job Failed");
      // since not emmitting remusCompletedNormally, won't
      // get previewDialog; thus need to to re-enable external process here
      emit enableExternalProcesses(true);
      }
    disconnect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)), 0, 0);
    }
  else if(newStatus)
  {
  //the message we get from the server is actually a multiple line
  //message for omicron.
  //this means that we have to spend some time and strip out the
  //last progress line to set as the message
  QString message = QString::fromStdString(status.progress().message());

  //Remus currently tacks on trailing and leading newline characters,so remove
  message = message.trimmed();
  if (message.size() > 0)
    {
    this->Internal->ProcessBar->appendToOutput(message);
    int index = message.lastIndexOf("Progress:");
    if(index >= 0)
      {
      int endOfLineIndex = message.indexOf('\n',index);
      int len = endOfLineIndex - index;
      this->Internal->ProcessBar->setMessage(
            message.midRef(index,len).toString());
      }
    }
  }
}


//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::killProcess()
{
  if (!this->Internal->MeshingClient)
    {
    return;
    }

  this->Internal->MeshingClient->terminateJob();

  this->Internal->ProcessTimer->stop();
  this->Internal->ProcessBar->enableAbort(false);

  this->Internal->ProcessBar->appendToOutput("User abort!");
  this->Internal->ProcessBar->setMessage("Process aborted!");

  emit enableExternalProcesses(true);
  disconnect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)), 0, 0);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::closeData()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqActiveObjects::instance().setActiveSource(NULL);
  pqActiveObjects::instance().setActivePort(NULL);

  builder->destroyLookupTables(this->Internal->ActiveServer);
  if(this->Internal->AppearanceEditor)
    {
    delete this->Internal->AppearanceEditor;
    this->Internal->AppearanceEditor = NULL;
    }
  if(this->Internal->RenderView)
    {
    this->Internal->RenderView->resetCamera();
    this->Internal->RenderView->render();
    }
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::getExistingFileName(const QString filters,
                                                 const QString title,
                                                 QString &selectedFile)
{
  pqFileDialog file_dialog(this->Internal->ActiveServer,
                           this->Internal->Parent, title, QString(), filters);
  file_dialog.setObjectName("FileOpenDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  if (file_dialog.exec() != QDialog::Accepted)
    {
    return false;
    }
  QStringList files = file_dialog.getSelectedFiles();
  if (files.size() == 0)
    {
    return false;
    }
  selectedFile = files[0];
  return true;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onHelpEnableTooltips(bool enabled)
{
  if(enabled)
    {
    delete this->Internal->ToolTipTrapper;
    this->Internal->ToolTipTrapper = 0;
    }
  else
    {
    this->Internal->ToolTipTrapper = new pqToolTipTrapper();
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onEditSettings()
{
  if(!this->Internal->AppSettingsDialog)
    {
    return;
    }

  this->Internal->AppSettingsDialog->show();
  this->Internal->AppSettingsDialog->raise();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onActiveViewChanged(pqView* view)
{
  pqRenderView* renderView = qobject_cast<pqRenderView*>(view);

  // Get the active source and server.
  pqServerManagerModelItem *item = this->getActiveObject();
  pqPipelineSource *source = dynamic_cast<pqPipelineSource *>(item);
  pqServer *server = this->getActiveServer();

  // Update the reset center action.
  emit this->enableResetCenter(source != 0 && renderView != 0);

  // Update the show center axis action.
  emit this->enableShowCenterAxis(renderView != 0);

  // Update the save screenshot action.
  emit this->enableFileSaveScreenshot(server != 0 && renderView != 0);

  // Update the view undo/redo state.
//  this->updateViewUndoRedo(renderView);
  if(renderView)
    {
    // Make sure the render module undo stack is connected.
    //this->connect(renderView, SIGNAL(canUndoChanged(bool)),
    //    this, SLOT(onActiveViewUndoChanged()));
    }

  if(this->Internal->CameraDialog && renderView)
    {
    this->showCameraDialog(renderView);
    }
}

//-----------------------------------------------------------------------------
pqServerManagerModelItem *pqCMBCommonMainWindowCore::getActiveObject() const
{
  return pqActiveObjects::instance().activeSource();
  // FIXME SEB
//  pqServerManagerModelItem *item = 0;
//  pqServerManagerSelectionModel *selection =
//      pqApplicationCore::instance()->getSelectionModel();
//  const pqServerManagerSelection *selected = selection->selectedItems();
//  if(selected->size() == 1)
//    {
//    item = selected->first();
//    }
//  else if(selected->size() > 1)
//    {
//    item = selection->currentItem();
//    if(item && !selection->isSelected(item))
//      {
//      item = 0;
//      }
//    }

//  return item;
}

namespace
{
  QStringList getLibraries(const QString& serverPath, pqServer* server)
    {
    QStringList libs;
    pqFileDialogModel model(server);
    model.setCurrentPath(serverPath);
    int numfiles = model.rowCount(QModelIndex());
    for(int i=0; i<numfiles; i++)
      {
      QModelIndex idx = model.index(i, 0, QModelIndex());
      QString file = model.getFilePaths(idx)[0];
      QFileInfo fileinfo(file);
      // if file names start with known lib suffixes
      if(fileinfo.completeSuffix().indexOf(
        QRegExp("(so|dll|sl|dylib)$")) == 0)
        {
          if(fileinfo.completeBaseName().indexOf(QRegExp("CMB_Plugin")) != -1 ||
            fileinfo.completeBaseName().indexOf(QRegExp("ModelBridge_Plugin")) != -1)
//            || fileinfo.completeBaseName().indexOf(QRegExp("SimBuilderMesh_Plugin")) != -1)
          {
          libs.append(file);
          }
        }
      }
    return libs;
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onServerCreationFinished(pqServer *server)
{
  pqApplicationCore *core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Internal->ActiveServer = server;

  this->Internal->RenderView = qobject_cast<pqRenderView*>(
    builder->createView(pqRenderView::renderViewType(),
    this->Internal->ActiveServer ));
  //vtkSMMultiProcessRenderView* mrv = vtkSMMultiProcessRenderView::SafeDownCast(
  //  this->Internal->RenderView->getProxy());

  //// Always remote render if there is a server.
  //if(mrv)
  //  {
  //  mrv->SetRemoteRenderThreshold(0.0);
  //  }

  this->Internal->RenderView->getProxy()->UpdateVTKObjects();

  pqActiveObjects::instance().blockSignals(true);
  pqActiveObjects::instance().setActiveView(this->Internal->RenderView);
  pqActiveObjects::instance().blockSignals(false);
  this->Internal->RenderViewSelectionHelper.setView(this->Internal->RenderView);
  this->selectionManager()->setActiveView(this->Internal->RenderView);

  QObject::disconnect(
    &pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
    this->selectionManager(), SLOT(setActiveView(pqView*)));


  // FIXME SEB
//  core->getSelectionModel()->blockSignals(true);
//  core->getSelectionModel()->setCurrentItem(server,
//    pqServerManagerSelectionModel::ClearAndSelect);
//  core->getSelectionModel()->blockSignals(false);
  this->Internal->RenderView->resetCamera();
  this->Internal->RenderView->render();
  this->Internal->RenderView->getWidget()->installEventFilter(this);

  // link in the cmb plugin here as all applications use it.
  PV_PLUGIN_IMPORT(CMB_Plugin)

  // Load the plugins for the server, paraview initializer code
  // handles the client side plugins
  pqPluginManager* pluginMgr = core->getPluginManager();

  // if we have a remote server find and load all plugins that we might use
  QString errormsg;
  if(this->Internal->ActiveServer->isRemote())
    {
    pqFileDialogModel model(this->Internal->ActiveServer);
    QString serverPath = model.getCurrentPath();
    QStringList libs = ::getLibraries(serverPath, this->Internal->ActiveServer);
    foreach(QString path, libs)
      {
      pluginMgr->loadExtension(this->Internal->ActiveServer,
        path, &errormsg);
      }
    }
  pluginMgr->hidePlugin("linked-in", false); // vtkPVInitializerPlugin

  this->setCenterAxesVisibility(false);
}
//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::eventFilter(QObject* caller, QEvent* e)
{
  if( qobject_cast<QVTKWidget*>(caller) && e->type() == QEvent::Resize)
    {
    // Update ViewPosition and GUISize properties on all view modules.
    this->updateViewPositions();
    }

  return QObject::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateViewPositions()
{
  // find a rectangle that bounds all views

  if(!this->activeRenderView())
    {
    return;
    }
  pqRenderView* view = this->activeRenderView();
  if (view->getWidget())
    {
    QRect bounds = view->getWidget()->rect();
    bounds.moveTo(view->getWidget()->mapToGlobal(QPoint(0,0)));
    int gui_size[2] = { bounds.width(), bounds.height() };
    // position relative to the bounds of all views
    QPoint view_pos = view->getWidget()->mapToGlobal(QPoint(0,0));
    view_pos -= bounds.topLeft();
    int position[2] = { view_pos.x(), view_pos.y() };
    vtkSMPropertyHelper(view->getProxy(), "ViewPosition", true).Set(position, 2);

    // size of each view.
    vtkSMPropertyHelper(view->getProxy(), "ViewSize", true).Set(gui_size, 2);
//    view->resetCamera();
    }
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBCommonMainWindowCore::getActiveSource()
{
  return qobject_cast<pqPipelineSource *>(this->getActiveObject());
}

//-----------------------------------------------------------------------------
pqServer* pqCMBCommonMainWindowCore::getActiveServer() const
{
  return this->Internal->ActiveServer;//.current();
}

//-----------------------------------------------------------------------------
pqRenderView* pqCMBCommonMainWindowCore::activeRenderView() const
{
  return this->Internal->RenderView;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::removeActiveSource()
{
  pqPipelineSource* source = this->getActiveSource();
  if (!source)
    {
    qDebug() << "No active source to remove.";
    return;
    }
  pqApplicationCore::instance()->getObjectBuilder()->destroy(source);
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBCommonMainWindowCore::createSourceOnActiveServer(
  const QString& /*xmlname*/)
{
  /*
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Internal->UndoStack->beginUndoSet(
    QString("Create '%1'").arg(xmlname));
  pqPipelineSource* source =
    builder->createSource("sources", xmlname, this->getActiveServer());
  this->Internal->UndoStack->endUndoSet();

  return source;
  */
  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetCamera()
{
  pqRenderView* ren = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  if (ren)
    {
    ren->resetCamera();
    ren->render();
    }
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetCameraManipulationMode()
{
  this->setCameraManipulationMode(
    this->Internal->PreviousCameraManipulationMode);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setCameraManipulationEnabled(bool enabled)
{
  this->Internal->CameraManipulationModeBox->setEnabled(enabled);
}
//-----------------------------------------------------------------------------
int pqCMBCommonMainWindowCore::getCameraManipulationMode()
{
  return this->Internal->CameraManipulationMode;
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setCameraManipulationMode(int mode)
{
  this->Internal->PreviousCameraManipulationMode =
    this->getCameraManipulationMode();
  this->Internal->CameraManipulationModeBox->setCurrentIndex(mode);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateCameraManipulationMode(int mode)
{
  if (mode == this->Internal->CameraManipulationMode)
    {
    return;
    }
  this->Internal->CameraManipulationMode = mode;
  if(this->Internal->CameraManipulationModeBox->currentIndex() != mode)
    {
    this->Internal->CameraManipulationModeBox->setCurrentIndex(mode);
    }

  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  std::string propertyName = "Camera3DManipulators";
  int interactorMode = vtkPVRenderView::INTERACTION_MODE_3D;

  const int *manipTypes;
  if (mode == pqCMBCommonMainWindowCore::vtkInternal::ThreeD)
    {
    manipTypes = &ThreeDManipulatorTypes[0];

    // turn OFF parallel projection
    pqSMAdaptor::setElementProperty(
      this->Internal->RenderView->getProxy()->GetProperty("CameraParallelProjection"), 0);
    this->Internal->VTKConnect->Disconnect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::MouseMoveEvent);
    this->Internal->VTKConnect->Disconnect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::EndInteractionEvent);
    this->Internal->VTKConnect->Disconnect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::LeaveEvent);
    this->Internal->VTKConnect->Disconnect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::EnterEvent);

    this->Internal->RenderView->render();
    emit cameraManipulationModeChangedTo3D();
    }
  else
    {
    manipTypes = &TwoDManipulatorTypes[0];
    propertyName="Camera2DManipulators";
    interactorMode = vtkPVRenderView::INTERACTION_MODE_2D;

    // turn ON parallel projection
    pqSMAdaptor::setElementProperty(
      this->Internal->RenderView->getProxy()->GetProperty("CameraParallelProjection"), 1);

    // mouse movement and end interaction event both result in update to
    // mouse position display;  mouse movement alone not enough, because
    // interaction (such as scroll zoom) could result in change of mouse
    // position without mouse movement
    this->Internal->VTKConnect->Connect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::MouseMoveEvent, this, SLOT(updateMousePositionText()));
    this->Internal->VTKConnect->Connect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::EndInteractionEvent, this, SLOT(updateMousePositionText()));

    // enter and leave events to cache whatever the current temporary message
    // is (enter), and put it back instead of mouse position upon leaving
    this->Internal->VTKConnect->Connect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::LeaveEvent, this, SLOT(leaveRenderView()));
    this->Internal->VTKConnect->Connect(
      this->Internal->RenderView->getRenderViewProxy()->GetInteractor(),
      vtkCommand::EnterEvent, this, SLOT(enterRenderView()));

    emit cameraManipulationModeChangedTo2D();
    }

  vtkSMProxy* viewproxy = this->Internal->RenderView->getProxy();
  vtkSMPropertyHelper( viewproxy->GetProperty(
    propertyName.c_str())).Set(manipTypes, 9);

  //viewproxy->UpdateProperty(propertyName.c_str(), 1);
  vtkSMPropertyHelper(viewproxy, "InteractionMode").Set(interactorMode);
  //viewproxy->UpdateProperty("InteractionMode",1);
  viewproxy->UpdateVTKObjects();

  this->updateCameraPositionDueToModeChange();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::enterRenderView()
{
  this->Internal->MessageBeforeEnterRenderView = "";
  if (this->Internal->StatusBar)
    {
    this->Internal->MessageBeforeEnterRenderView =
      this->Internal->StatusMessage->text();
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::leaveRenderView()
{
  if (this->Internal->StatusBar)
    {
    this->Internal->Position1Message->setText("");
    this->Internal->Position2Message->setText("");
    this->showStatusMessage(
      this->Internal->MessageBeforeEnterRenderView );
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::showStatusMessage(const QString& strMessage)
{
  this->Internal->StatusMessage->setText(strMessage);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateMousePositionText()
{
  if (!this->Internal->StatusBar)
    {
    // if status bar not set, means we didn't setup the mouse position display
    return;
    }

  int eventPosition[2];
  this->Internal->RenderView->getRenderViewProxy()->GetInteractor()->GetEventPosition(
    eventPosition );
  QSize wSize = this->Internal->RenderView->getSize();

  // Compute index with center as origin (0.5 offset because we want location at
  // center of pixel).
  double centerIndex[2];
  centerIndex[0] = (eventPosition[0] + 0.5) - static_cast<double>(wSize.width()) * 0.5;
  centerIndex[1] = (eventPosition[1] + 0.5) - static_cast<double>(wSize.height()) * 0.5;

  double focalPt[3], position[3], viewUp[3], viewRight[3], viewDirection[3];
  double cameraDistance, parallelScale;
  this->getCameraInfo(focalPt, position, viewDirection, cameraDistance,
    viewUp, parallelScale);

  // compute viewRight
  vtkMath::Cross(viewDirection, viewUp, viewRight);

  // calculate "spacing"... which is same horizontally and vertically
  // see vtkCamera::ComputeProjectionTransform for basis of conversion
  // from ParallelScale to spacing
  double spacing = (2.0 * parallelScale) / static_cast<double>(wSize.height());

  // focalPt = center
  double point[3];
  for (int i = 0; i < 3; i++)
    {
    point[i] = focalPt[i]
      + viewRight[i]*centerIndex[0]*spacing
      + viewUp[i]*centerIndex[1]*spacing;
    }

  QString pos1Message, pos2Message;
  if (viewDirection[0] < -.99 || viewDirection[0] > .99)
    {
    pos1Message = "Y: " + QString::number(point[1]);
    pos2Message = "Z: " + QString::number(point[2]);
    }
  else if (viewDirection[1] < -.99 || viewDirection[1] > .99)
    {
    pos1Message = "X: " + QString::number(point[0]);
    pos2Message = "Z: " + QString::number(point[2]);
    }
  else
    {
    pos1Message = "X: " + QString::number(point[0]);
    pos2Message = "Y: " + QString::number(point[1]);
    }

  //this->Internal->StatusBar->clearMessage();
  this->Internal->Position1Message->setText(pos1Message);
  this->Internal->Position2Message->setText(pos2Message);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirection(
    double look_x, double look_y, double look_z,
    double up_x, double up_y, double up_z)
{
  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;
  this->getCameraInfo(focalPt, position, viewDirection, cameraDistance,
    viewUp, parallelScale);

  QList<QVariant> values;
  values << focalPt[0] - look_x * cameraDistance;
  values << focalPt[1] - look_y * cameraDistance;
  values << focalPt[2] - look_z * cameraDistance;

  pqSMAdaptor::setMultipleElementProperty(
    this->Internal->RenderView->getProxy()->GetProperty("CameraPosition"), values);

  values.clear();
  values << up_x << up_y << up_z;
  pqSMAdaptor::setMultipleElementProperty(
    this->Internal->RenderView->getProxy()->GetProperty("CameraViewUp"), values);

  this->Internal->RenderView->getProxy()->UpdateVTKObjects();

  this->Internal->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionPosX()
{
  this->resetViewDirection(1, 0, 0, 0, 0, 1);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionNegX()
{
  this->resetViewDirection(-1, 0, 0, 0, 0, 1);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionPosY()
{
  this->resetViewDirection(0, 1, 0, 0, 0, 1);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionNegY()
{
  this->resetViewDirection(0, -1, 0, 0, 0, 1);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionPosZ()
{
  this->resetViewDirection(0, 0, 1, 0, 1, 0);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetViewDirectionNegZ()
{
  this->resetViewDirection(0, 0, -1, 0, 1, 0);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::getCameraInfo(double focalPt[3],
                                       double position[3],
                                       double viewDirection[3],
                                       double &distance,
                                       double viewUp[3],
                                       double &parallelScale)
{
  this->getViewCameraInfo(this->Internal->RenderView, focalPt, position,
    viewDirection, distance, viewUp, parallelScale);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::getViewCameraInfo(pqRenderView* view,
  double focalPt[3], double position[3],
  double viewDirection[3], double &distance, double viewUp[3],
  double &parallelScale)
{
  QList<QVariant> values;
  values = pqSMAdaptor::getMultipleElementProperty(
      view->getProxy()->GetProperty("CameraFocalPointInfo"));
  focalPt[0] = values[0].toDouble();
  focalPt[1] = values[1].toDouble();
  focalPt[2] = values[2].toDouble();
  values = pqSMAdaptor::getMultipleElementProperty(
      view->getProxy()->GetProperty("CameraPositionInfo"));
  position[0] = values[0].toDouble();
  position[1] = values[1].toDouble();
  position[2] = values[2].toDouble();

  viewDirection[0] = focalPt[0] - position[0];
  viewDirection[1] = focalPt[1] - position[1];
  viewDirection[2] = focalPt[2] - position[2];
  distance = vtkMath::Normalize(viewDirection);

  parallelScale = pqSMAdaptor::getElementProperty(
    view->getProxy()->GetProperty("CameraParallelScaleInfo")).toDouble();

  values = pqSMAdaptor::getMultipleElementProperty(
    view->getProxy()->GetProperty("CameraViewUpInfo"));
  viewUp[0] = values[0].toDouble();
  viewUp[1] = values[1].toDouble();
  viewUp[2] = values[2].toDouble();
}


//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateCameraPositionDueToModeChange()
{
  QList<QVariant> values;
  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;

  this->getCameraInfo(focalPt, position, viewDirection, cameraDistance,
    viewUp, parallelScale);

  // switching between 3D and 2D modes, while keeping approximately the same
  // view, is done by adjusting camera position (and thus camera distance)
  // based on 2D parallelScale when going from 2D to 3D, and adjusitng
  // parallelScale based on camera distance when going from 3D to 2D.  The
  // focal point remains unchanged.   A factor of 4 (didn't come up with it
  // empirically) is used to convert between distance and parallScale.
  // Seems to give "good" results much of the time, but ultimately it does
  // depend on what the actual focal point is and it may not be what the user
  // thinks it is; might either be closer or farther than point of interest.
  // Thus (?) sometimes it is likely to not do what user wants.
  if (this->Internal->CameraManipulationMode == pqCMBCommonMainWindowCore::vtkInternal::ThreeD)
    {
    cameraDistance = 4 * parallelScale;
    }
  else
    {
    // compute viewRight which we'll use to compute new viewUp
    double viewRight[3];
    vtkMath::Cross(viewDirection, viewUp, viewRight);

    // component of viewDirection with largest magnitude is the closest 2D view direction
    int majorComponent = 0;
    if (fabs(viewDirection[1]) > fabs(viewDirection[0]))
      {
      majorComponent = 1;
      }
    if (fabs(viewDirection[2]) > fabs(viewDirection[majorComponent]))
      {
      majorComponent = 2;
      }

    double valueOfMaxComponent = viewDirection[majorComponent];
    viewDirection[0] = viewDirection[1] = viewDirection[2] = 0;
    viewDirection[majorComponent] = (valueOfMaxComponent < 0) ? -1 : 1;

    // compute new viewUp
    vtkMath::Cross(viewRight, viewDirection, viewUp);
    values.clear();
    values << viewUp[0] << viewUp[1] << viewUp[2];
    pqSMAdaptor::setMultipleElementProperty(
      this->Internal->RenderView->getProxy()->GetProperty("CameraViewUp"), values);

    pqSMAdaptor::setElementProperty(
      this->Internal->RenderView->getProxy()->GetProperty("CameraParallelScale"), cameraDistance/4.0 );
    }

  // set the position of the camera, regardless of whether in 2D or 3D
  position[0] = focalPt[0] - viewDirection[0] * cameraDistance;
  position[1] = focalPt[1] - viewDirection[1] * cameraDistance;
  position[2] = focalPt[2] - viewDirection[2] * cameraDistance;
  values.clear();
  values << position[0] << position[1] << position[2];
  pqSMAdaptor::setMultipleElementProperty(
    this->Internal->RenderView->getProxy()->GetProperty("CameraPosition"), values);

  this->Internal->RenderView->getProxy()->UpdateVTKObjects();

  this->Internal->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::enableTestingRenderWindowSize(bool enable)
{
  this->setMaxRenderWindowSize(
    enable? QSize(300, 300) : QSize(-1, -1));
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setMaxRenderWindowSize(const QSize& /*size*/)
{
  //this->Internal->MultiViewManager.setMaxViewWindowSize(size);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::pickCenterOfRotation(bool begin)
{
 if (!qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView()))
    {
    return;
    }

  if (begin)
    {
    this->Internal->RenderViewSelectionHelper.beginFastIntersect();
    }
  else
    {
    this->Internal->RenderViewSelectionHelper.endPick();
    }
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::linkCenterWithFocalPoint(bool linked)
{
  this->Internal->CenterFocalLinked = linked;
//  if(linked)
//  {
//    this->updateFocalPointWithCenter();
//  }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateFocalPointWithCenter()
{
  double pos[3];
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(
    this->activeRenderView()->getRenderViewProxy()->GetProperty("CameraFocalPoint"));
  pos[0] = values[0].toDouble();
  pos[1] = values[1].toDouble();
  pos[2] = values[2].toDouble();
  double center[3];
  this->activeRenderView()->getCenterOfRotation(center);
  if(center[0]==pos[0] && center[1]==pos[1] && center[2]==pos[2])
    {
    return;
    }

  double position[3], focalPt[3];

  vtkSMRenderViewProxy* proxy = this->activeRenderView()->getRenderViewProxy();
  vtkSMPropertyHelper(proxy, "CameraPosition").Get(position, 3);
  vtkSMPropertyHelper(proxy, "CameraFocalPoint").Get(focalPt, 3);

  // move position same as we move the focalPt
  double delta[3] = {center[0] - focalPt[0],
    center[1] - focalPt[1],
    center[2] - focalPt[2]};
  if (this->Internal->CameraManipulationMode == pqCMBCommonMainWindowCore::vtkInternal::TwoD)
    {
    // only "pan" the camera if in 2D manipulation mode...  axpecting that
    // all but one axis are same for position and focalPt... change those
    // that are the same.
    for (int i = 0; i < 3; i++)
      {
      if (focalPt[i] != position[i])
        {
        delta[i] = 0;
        }
      }
    }

  position[0] += delta[0];
  position[1] += delta[1];
  position[2] += delta[2];

  vtkSMPropertyHelper(proxy, "CameraPosition").Set(position, 3);
  vtkSMPropertyHelper(proxy, "CameraFocalPoint").Set(center, 3);
  proxy->UpdateVTKObjects();
  this->activeRenderView()->render();
}


//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::pickCenterOfRotationFinished(double x, double y, double z)
{
  //this->Internal->RenderViewSelectionHelper.endPick();
  emit this->pickingCenter(false);
  this->setCenterOfRotation(x, y, z);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setCenterOfRotation(double x, double y, double z)
{
  pqRenderView* rm = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  if (!rm)
    {
    qDebug() << "No active render module. Cannot reset center of rotation.";
    return;
    }

  double center[3];
  center[0] = x;
  center[1] = y;
  center[2] = z;

  rm->setCenterOfRotation(center);

  if(this->Internal->CenterFocalLinked)
    {
    double position[3], focalPt[3];

    vtkSMRenderViewProxy* proxy = rm->getRenderViewProxy();
    vtkSMPropertyHelper(proxy, "CameraPosition").Get(position, 3);
    vtkSMPropertyHelper(proxy, "CameraFocalPoint").Get(focalPt, 3);

    // move position same as we move the focalPt
    double delta[3] = {center[0] - focalPt[0],
      center[1] - focalPt[1],
      center[2] - focalPt[2]};
    if (this->Internal->CameraManipulationMode == pqCMBCommonMainWindowCore::vtkInternal::TwoD)
      {
      // only "pan" the camera if in 2D manipulation mode...  axpecting that
      // all but one axis are same for position and focalPt... change those
      // that are the same.
      for (int i = 0; i < 3; i++)
        {
        if (focalPt[i] != position[i])
          {
          delta[i] = 0;
          }
        }
      }
    position[0] += delta[0];
    position[1] += delta[1];
    position[2] += delta[2];

    vtkSMPropertyHelper(proxy, "CameraPosition").Set(position, 3);
    vtkSMPropertyHelper(proxy, "CameraFocalPoint").Set(center, 3);

    proxy->UpdateVTKObjects();
    }
  rm->render();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::resetCenterOfRotationToCenterOfCurrentData()
{
  this->resetCamera();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setCenterAxesVisibility(bool visible)
{
  pqRenderView* rm = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  if (!rm)
    {
    qDebug() << "No active render module. setCenterAxesVisibility failed.";
    return;
    }
  rm->setCenterAxesVisibility(visible);
  rm->render();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onToolsManageLinks()
{
  if(this->Internal->LinksManager)
    {
    this->Internal->LinksManager->raise();
    this->Internal->LinksManager->activateWindow();
    }
  else
    {
    this->Internal->LinksManager = new
      pqLinksManager(this->Internal->Parent);
    this->Internal->LinksManager->setWindowTitle("Link Manager");
    this->Internal->LinksManager->setAttribute(Qt::WA_DeleteOnClose);
    this->Internal->LinksManager->show();
    }
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onSaveScreenshot()
{
  if(!qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView()))
    {
    qDebug() << "Cannot save image. No active render module.";
    return;
    }

  QString filters;
  filters += "PNG Image (*.png)";
  filters += ";;BMP Image (*.bmp)";
  filters += ";;TIFF Image (*.tif)";
  filters += ";;PPM Image (*.ppm)";
  filters += ";;JPG Image (*.jpg)";
  filters += ";;All Files (*)";
  pqFileDialog fileDialog (NULL,
      this->Internal->Parent, tr("Save Test Screenshot"), QString(),
      filters);
  //fileDialog->setAttribute(Qt::WA_DeleteOnClose);
  fileDialog.setObjectName("RecordTestScreenshotDialog");
  fileDialog.setFileMode(pqFileDialog::AnyFile);
  QObject::connect(&fileDialog, SIGNAL(filesSelected(const QStringList &)),
      this, SLOT(onSaveScreenshot(const QStringList &)));
  fileDialog.exec();
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onSaveScreenshot(const QStringList &fileNames)
{
  pqRenderView* const render_module = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  if(!render_module)
    {
    qCritical() << "Cannnot save image. No active render module.";
    return;
    }

  QVTKWidget* const widget =
    qobject_cast<QVTKWidget*>(render_module->getWidget());
  assert(widget);

//  this->Internal->isComparingScreenImage = true;

  QSize old_size = widget->size();
  widget->resize(300,300);

  QStringList::ConstIterator iter = fileNames.begin();
  for( ; iter != fileNames.end(); ++iter)
    {
    vtkSmartPointer<vtkImageData> img;
    img.TakeReference(render_module->captureImage(widget->size()));
    if (img.GetPointer() == NULL)
      {
      qCritical() << "Save Image failed.";
      }
    else
      {
      pqImageUtil::saveImage(img, *iter, 100);
      }
    }

  widget->resize(old_size);
  render_module->render();
//  this->Internal->isComparingScreenImage = false;
}

//-----------------------------------------------------------------------------
pqUndoStack* pqCMBCommonMainWindowCore::getApplicationUndoStack() const
{
  return this->Internal->UndoStack;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::applicationInitialize()
{
  /*pqApplicationCore* core = */pqApplicationCore::instance();
  pqOptions* options = pqOptions::SafeDownCast(
    vtkProcessModule::GetProcessModule()->GetOptions());

  // check for --server.
  const char* serverresource_name = options->GetServerResourceName();
  if (serverresource_name)
    {
    if (!pqServerConnectReaction::connectToServerUsingConfigurationName(
        serverresource_name))
      {
      qCritical() << "Could not connect to requested server \""
        << serverresource_name
        << "\". Creating default builtin connection.";
      }
    }

  if (!this->getActiveServer())
    {
    this->makeDefaultConnectionIfNoneExists();
    }

  // Set Data directory For testing
  options->SetDataDirectory(CMB_TEST_DATA_ROOT);

  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqCMBTreeWidgetEventTranslator(
                      pqApplicationCore::instance()->testUtility() ));

  this->InitializePythonEnvironment();

}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::showCameraDialog(pqView* view)
{
  if(!view)
    {
    if(this->Internal->CameraDialog)
      {
      this->Internal->CameraDialog->SetCameraGroupsEnabled(false);
      }
    return;
    }
  pqRenderView* renModule = qobject_cast<pqRenderView*>(view);

  if (!renModule)
    {
    if(this->Internal->CameraDialog)
      {
      this->Internal->CameraDialog->SetCameraGroupsEnabled(false);
      }
    return;
    }

  if(!this->Internal->CameraDialog)
    {
    this->Internal->CameraDialog = new pqCameraDialog(
      this->Internal->Parent);
    this->Internal->CameraDialog->setWindowTitle("Adjust Camera");
    this->Internal->CameraDialog->setAttribute(Qt::WA_DeleteOnClose);
    this->Internal->CameraDialog->setRenderModule(renModule);
    this->Internal->CameraDialog->show();
    }
  else
    {
    this->Internal->CameraDialog->SetCameraGroupsEnabled(true);
    this->Internal->CameraDialog->setRenderModule(renModule);
    this->Internal->CameraDialog->raise();
    this->Internal->CameraDialog->activateWindow();
    }

}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::fiveMinuteTimeoutWarning()
{
  QMessageBox::warning(this->Internal->Parent,
    tr("Server Timeout Warning"),
    tr("The server connection will timeout under 5 minutes.\n"
    "Please save your work."),
    QMessageBox::Ok);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::finalTimeoutWarning()
{
  QMessageBox::critical(this->Internal->Parent,
    tr("Server Timeout Warning"),
    tr("The server connection will timeout shortly.\n"
    "Please save your work."),
    QMessageBox::Ok);
}

//-----------------------------------------------------------------------------
// update the state of the \c node if node is not an ancestor of any of the
// non-blockable widgets. If so, then it recurses over all its children.
static void selectiveEnabledInternal(QWidget* node,
  QList<QPointer<QObject> >& nonblockable, bool enable)
{
  if (!node)
    {
    return;
    }
  if (nonblockable.size() == 0)
    {
    node->setEnabled(enable);
    return;
    }

  foreach (QObject* objElem, nonblockable)
    {
    QWidget* elem = qobject_cast<QWidget*>(objElem);
    if (elem)
      {
      if (node == elem)
        {
        // this is a non-blockable wiget. Don't change it's enable state.
        nonblockable.removeAll(elem);
        return;
        }

      if (node->isAncestorOf(elem))
        {
        // iterate over all children and selectively disable each.
        QList<QObject*> children = node->children();
        for (int cc=0; cc < children.size(); cc++)
          {
          QWidget* child = qobject_cast<QWidget*>(children[cc]);
          if (child)
            {
            ::selectiveEnabledInternal(child, nonblockable, enable);
            }
          }
        return;
        }
      }
    }

  // implies node is not an ancestor of any of the nonblockable widgets,
  // we can simply update its enable state.
  node->setEnabled(enable);
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setSelectiveEnabledState(bool enable)
{
  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();
  QList<QPointer<QObject> > nonblockable = progress_manager->nonBlockableObjects();

  if (nonblockable.size() == 0)
    {
    this->Internal->Parent->setEnabled(enable);
    return;
    }

  // Do selective disbling.
  selectiveEnabledInternal(this->Internal->Parent, nonblockable, enable);
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::requestRender()
{
  this->Internal->RenderView->render();
}

//-----------------------------------------------------------------------------
pqPipelineSource *pqCMBCommonMainWindowCore::getAppendedSource(QList<pqOutputPort*> &inputs)
{
  if(inputs.count() == 0)
    {
    return 0;
    }

  pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", this->getActiveServer());
  if (inputs.count() == 1)
    {
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast( inputs[0]->getSource()->getProxy() ));
    }
  else
    {
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = inputs;

    pqPipelineSource* appendPoly = builder->createFilter(
      "filters", "AppendPolyData", namedInputs, this->getActiveServer());
    appendPoly->updatePipeline();

    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(appendPoly->getProxy()));
    builder->destroy( appendPoly );
    }
  return pdSource;
}

//-----------------------------------------------------------------------------
pqContourWidget* pqCMBCommonMainWindowCore::createPqContourWidget(int& orthoPlane)
{
  // We need to explicitly call this to make sure the mode is 2D
  this->setCameraManipulationMode(1);
  this->setCameraManipulationEnabled(false);

  this->Internal->RenderView->forceRender();

  pqContourWidget* contourWidget = this->createDefaultContourWidget();

  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;
  this->getCameraInfo(focalPt, position, viewDirection, cameraDistance,
    viewUp, parallelScale);
  orthoPlane = 2; // z axis
  if (viewDirection[0] < -.99 || viewDirection[0] > .99)
    {
    orthoPlane = 0; // x axis
    }
  else if (viewDirection[1] < -.99 || viewDirection[1] > .99)
    {
    orthoPlane = 1; // y axis;
    }

  QList<QVariant> values =
  pqSMAdaptor::getMultipleElementProperty(
                                          this->activeRenderView()->getProxy()->GetProperty("CameraFocalPointInfo"));
  double projpos = values[orthoPlane].toDouble();;
  values =
  pqSMAdaptor::getMultipleElementProperty(
                                          this->activeRenderView()->getProxy()->GetProperty("CameraClippingRange"));

  double small_offset = (values[1].toDouble() - values[0].toDouble())*0.01;
  double direction = viewDirection[orthoPlane];
  double min, max;
  if(direction <0)
  {
    min = position[orthoPlane] - values[1].toDouble() + small_offset;
    max = position[orthoPlane] - values[0].toDouble() - small_offset;
  }
  else
  {
    min = position[orthoPlane] + values[0].toDouble() + small_offset;
    max = position[orthoPlane] + values[1].toDouble() - small_offset;
  }
  if(projpos <= min)
  {
    projpos =  position[orthoPlane] + viewDirection[orthoPlane]*((values[1].toDouble() + values[0].toDouble())*0.5);
  }
  else if(projpos >= max)
  {
    projpos =  position[orthoPlane] + viewDirection[orthoPlane]*((values[1].toDouble() + values[0].toDouble())*0.5);
  }
  //   pqSMAdaptor::getMultipleElementProperty(
  //     this->activeRenderView()->getProxy()->GetProperty("CameraClippingRange"));
  //double projpos = position[orthoPlane] + viewDirection[orthoPlane]*((values[1].toDouble() + values[0].toDouble())*0.5);

  this->setContourPlane(contourWidget, orthoPlane, projpos);

  vtkSMPropertyHelper(contourWidget->getWidgetProxy(), "AlwaysOnTop").Set(1);

  contourWidget->setVisible(0);
  contourWidget->setEnabled(0);

  contourWidget->setWidgetVisible(1);
  vtkSMPropertyHelper(contourWidget->getWidgetProxy(), "Enabled").Set(1);
  contourWidget->getWidgetProxy()->UpdateVTKObjects();
  contourWidget->showWidget();

  return contourWidget;
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::deleteContourWidget(
  pqContourWidget* contourWidget)
{
  if(contourWidget)
    {
    vtkSMNewWidgetRepresentationProxy* widget = contourWidget->getWidgetProxy();
    if(widget)
      {
      vtkSMProxyManager* pxm=vtkSMProxyManager::GetProxyManager();
      pxm->UnRegisterProxy("3d_widgets_prototypes",
        pxm->GetProxyName("3d_widgets_prototypes", widget),widget);
      }
    delete contourWidget;
    }
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::updateContourLoop(
  vtkSMProxy* implicitLoop, pqContourWidget* contourWidgt)
{
  // force read
  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    contourWidgt->getWidgetProxy()->GetRepresentationProxy());
  if(repProxy)
  {
    repProxy->UpdateVTKObjects();
    repProxy->UpdatePipeline();
  }

  vtkNew<vtkPVContourRepresentationInfo> contourInfo;
  contourWidgt->getWidgetProxy()->GetRepresentationProxy()->GatherInformation(
              contourInfo.GetPointer());

  if(contourInfo->GetNumberOfAllNodes())
  {
    // Points proxy
    vtkSMProxy* loopPoints = vtkSMProxyManager::GetProxyManager()->NewProxy(
      "points", "Points");
    QList<QVariant> values;
    vtkDoubleArray* nodeArray = contourInfo->GetAllNodesWorldPositions();
    double pos[3];
    for(vtkIdType i=0; i<nodeArray->GetNumberOfTuples(); i++)
    {
      nodeArray->GetTuple(i, pos);
      values << pos[0] << pos[1] << pos[2];
    }
    pqSMAdaptor::setMultipleElementProperty(loopPoints->GetProperty("Points"), values);
    loopPoints->UpdateVTKObjects();
    // ImplicitSelectionLoop Proxy
    vtkSMProxyProperty* selectionLoop =
      vtkSMProxyProperty::SafeDownCast(
      implicitLoop->GetProperty("Loop"));
    selectionLoop->RemoveAllProxies();
    selectionLoop->AddProxy(loopPoints);

    double normal[3];
    if(this->getContourNormal(normal, contourWidgt))
    {
      vtkSMPropertyHelper(implicitLoop, "Normal").Set(normal, 3);
    }

    implicitLoop->UpdateVTKObjects();

    loopPoints->Delete();
  }
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::getContourNormal(double normal[3],
  pqContourWidget* contourWidget)
{
  int orthoPlane;
  if(this->getContourProjectionNormal(orthoPlane, contourWidget))
  {
    if (orthoPlane == 0)// x axis
    {
      normal[0] = 1.0;normal[1]=0.0; normal[2]=0.0;
    }
    else if (orthoPlane == 1)// y axis;
    {
      normal[0] = 0.0;normal[1]=1.0; normal[2]=0.0;
    }
    else if(orthoPlane == 2)
    {
      normal[0] = 0.0;normal[1]=0.0; normal[2]=1.0;
    }
    else
    {
      return false;
    }
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::getContourProjectionNormal(int &projNormal,
                                          pqContourWidget* contourWidget)
{
  vtkSMNewWidgetRepresentationProxy* widget = contourWidget->getWidgetProxy();
  if (widget)
    {
    vtkSMProxyProperty* proxyProp =
      vtkSMProxyProperty::SafeDownCast(
      widget->GetProperty("PointPlacer"));
    if (proxyProp && proxyProp->GetNumberOfProxies())
      {
      projNormal = pqSMAdaptor::getElementProperty(
        proxyProp->GetProxy(0)->GetProperty("ProjectionNormal")).toInt();
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::getContourProjectionPosition(double &position,
                                          pqContourWidget* contourWidget)
{
  vtkSMNewWidgetRepresentationProxy* widget = contourWidget->getWidgetProxy();
  if (widget)
    {
    vtkSMProxyProperty* proxyProp =
      vtkSMProxyProperty::SafeDownCast(
      widget->GetProperty("PointPlacer"));
    if (proxyProp && proxyProp->GetNumberOfProxies())
      {
      position = pqSMAdaptor::getElementProperty(
        proxyProp->GetProxy(0)->GetProperty("ProjectionPosition")).toDouble();
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
pqContourWidget* pqCMBCommonMainWindowCore::createContourWidgetFromSource(
  int orthoplane, double projPos, vtkSMSourceProxy* source)
{
  pqContourWidget* contourWidget = this->createDefaultContourWidget();
  this->setContourPlane(contourWidget, orthoplane, projPos);

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource *pdSource = builder->createSource("sources",
    "SceneContourSource", this->getActiveServer());
  vtkSMSceneContourSourceProxy* dwProxy =
    vtkSMSceneContourSourceProxy::SafeDownCast(pdSource->getProxy());
  dwProxy->ExtractContour( source );
  dwProxy->UpdatePipeline();
  bool closed=false;
  dwProxy->EditData( contourWidget->getWidgetProxy(), closed );
  dwProxy->UpdatePipeline();

  vtkSMPropertyHelper(contourWidget->getWidgetProxy(), "AlwaysOnTop").Set(1);
  contourWidget->setVisible(0);
  contourWidget->setEnabled(0);

  contourWidget->setWidgetVisible(1);
  vtkSMPropertyHelper(contourWidget->getWidgetProxy(), "Enabled").Set(1);
  vtkSMPropertyHelper(contourWidget->getWidgetProxy(), "ShowSelectedNodes").Set(0);
  contourWidget->getWidgetProxy()->UpdateVTKObjects();
  contourWidget->showWidget();

  return contourWidget;
}
//-----------------------------------------------------------------------------
pqContourWidget* pqCMBCommonMainWindowCore::createDefaultContourWidget()
{
  vtkSMProxy* pointplacer = vtkSMProxyManager::GetProxyManager()->NewProxy(
    "point_placers", "BoundedPlanePointPlacer");
  pqContourWidget* contourWidget = new pqContourWidget(
    pointplacer, pointplacer, NULL);
  contourWidget->setObjectName("LIDARContourWidget");
  contourWidget->setView(this->activeRenderView());
  contourWidget->setPointPlacer(pointplacer);
  this->activeRenderView()->getProxy()->UpdateVTKObjects();
  contourWidget->setLineInterpolator(0);
  pointplacer->Delete();
  return contourWidget;
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setContourPlane(pqContourWidget* contourWidget,
  int orthoPlane, double projpos)
{
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(
    contourWidget->getWidgetProxy()->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
    {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(orthoPlane);
    vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(projpos);
    pointplacer->MarkModified(pointplacer);
    pointplacer->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
qtCMBApplicationOptions* pqCMBCommonMainWindowCore::cmbAppOptions()
{
  if ( !this->Internal->CmbAppOptions )
    {
    // this will load all settings defined in the ini file if it exists
    this->Internal->CmbAppOptions = new qtCMBApplicationOptions(
      this->Internal->AppSettingsDialog);
    }
  return this->Internal->CmbAppOptions;
}

//-----------------------------------------------------------------------------
qtCMBApplicationOptionsDialog* pqCMBCommonMainWindowCore::appSettingsDialog()
{
  if(!this->Internal->AppSettingsDialog)
    {
    this->Internal->AppSettingsDialog =
      new qtCMBApplicationOptionsDialog(this->Internal->Parent);
    this->Internal->AppSettingsDialog->setObjectName("ApplicationSettings");
    this->Internal->AppSettingsDialog->setAttribute(Qt::WA_QuitOnClose, false);
    this->Internal->AppSettingsDialog->addOptions("CMB Suite", this->cmbAppOptions());
    QStringList pages = this->cmbAppOptions()->getPageList();
    if(pages.size())
      {
      this->Internal->AppSettingsDialog->setCurrentPage(pages[0]);
      }

    this->connect(Internal->AppSettingsDialog, SIGNAL(appliedChanges()),
      this, SLOT(applyAppSettings()));
    }
  return this->Internal->AppSettingsDialog;
}
//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::applyAppSettings()
{
  this->appSettingsDialog();
  //this->cmbAppOptions()->loadGlobalPropertiesFromSettings();
}


//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindowCore::InitializePythonEnvironment()
{
    if(!vtkPythonInterpreter::IsInitialized())
    {
        // If someone already initialized Python before ProcessModule was started,
        // we don't finalize it when ProcessModule finalizes. This is for the cases
        // where ParaView modules are directly imported in python (not pvpython).
        //pqCMBCommonMainWindowCore::FinalizePython = true;
        //this->FinalizePython = true;
        this->Internal->FinalizePython = true;
    }

    std::string program_path = QApplication::applicationFilePath().toStdString();
    std::string program_dir = QApplication::applicationDirPath().toStdString();
    std::string program_name = QApplication::applicationName().toStdString();

    vtkPythonInterpreter::SetProgramName(program_name.c_str());
    vtkCMBPythonAppInitPrependPath(program_dir.c_str());
    return true;
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::setDisplayRepresentation(pqDataRepresentation* rep)
{
  if(this->Internal->AppearanceEditor &&
     this->Internal->AppearanceEditor->displayRepresentation() == rep)
    {
    return;
    }
  else
    {
    pqOutputPort* actPort = rep ? rep->getOutputPortFromInput() : NULL;
    pqActiveObjects::instance().setActivePort(actPort);

    pqCMBDisplayProxyEditor* displayEditor = new pqCMBDisplayProxyEditor(
      rep, this->getAppearanceEditorContainer()->layout()->parentWidget());
    QCheckBox* visibleCheck = displayEditor->findChild<QCheckBox*>("ViewData");
    if(visibleCheck)
      {
      visibleCheck->hide();
      }
    QCheckBox* pickableCheck = displayEditor->findChild<QCheckBox*>("Selectable");
    if(pickableCheck)
      {
      pickableCheck->hide();
      }
    displayEditor->setObjectName("DisplayPropertyEditor");
    this->getAppearanceEditorContainer()->layout()->addWidget(
      displayEditor);
    displayEditor->setView(this->activeRenderView());
    displayEditor->setApplyChangesImmediately(true);
    displayEditor->filterWidgets(true);
    pqActiveObjects::instance().disconnect(displayEditor);

    QObject::connect(displayEditor, SIGNAL(changeFinished()),
      this, SLOT(requestRender()));

    this->setAppearanceEditor(displayEditor);
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindowCore::onViewCreated(pqView* view)
{
  // we need to make sure the current in pqActiveView instance is always
  // the RenderView in our main window. Otherwise, the paraview's features,
  // such as pqCoreTestUtility::compareView, or selectionManager, etc. will
  // not work. And paraview will automatically set the activeView to the newly
  // created view, which is not desired in CMB.
  if(view != this->Internal->RenderView)
    {
    pqActiveObjects::instance().setActiveView(this->Internal->RenderView);
    }
}
