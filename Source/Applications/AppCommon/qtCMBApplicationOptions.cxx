//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBApplicationOptions.h"
#include "ui_qtCMBApplicationOptions.h"

#include "pqActiveObjects.h"
#include "pqAnimationScene.h"
#include "pqApplicationCore.h"
#include "pqFileDialog.h"
#include "pqInterfaceTracker.h"
#include "pqObjectBuilder.h"
#include "pqOptions.h"
#include "pqPipelineRepresentation.h"
#include "pqPluginManager.h"
#include "pqProxyWidget.h"
#include "pqRenderView.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqScalarsToColors.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSettings.h"

#include "vtkNew.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLParser.h"
#include "vtkProcessModule.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSettings.h"
#include "vtkSMTransferFunctionPresets.h"

#include <QDir>
#include <QDoubleValidator>
#include <QMenu>
#include <QScrollArea>
#include <QTemporaryFile>
#include <sstream>

class qtCMBApplicationOptions::pqInternal : public Ui::qtCMBApplicationOptions
{
public:
  pqInternal() {}

  QPointer<pqSettings> CMBCommonSettings;
  QList<pqProxyWidget*> ProxyWidgets;
};

qtCMBApplicationOptions* qtCMBApplicationOptions::Instance = 0;

qtCMBApplicationOptions* qtCMBApplicationOptions::instance()
{
  return qtCMBApplicationOptions::Instance;
}

qtCMBApplicationOptions::qtCMBApplicationOptions(QWidget* widgetParent)
  : qtCMBOptionsContainer(widgetParent)
{
  // Only 1 qtCMBApplicationOptions instance can be created.
  Q_ASSERT(qtCMBApplicationOptions::Instance == NULL);
  qtCMBApplicationOptions::Instance = this;

  this->Internal = new pqInternal;
  this->Internal->setupUi(this);

  // Load in color table presets
  this->loadBuiltinColorPresets();

  QList<vtkSMProxy*> proxies_to_show;
  // Add RenderView proxy
  //  pqRenderView* ren = qobject_cast<pqRenderView*>(
  //    pqActiveObjects::instance().activeView());
  //  if (ren && ren->getViewProxy())
  //    {
  //    proxies_to_show.push_back(ren->getViewProxy());
  //    }

  pqServer* server = pqActiveObjects::instance().activeServer();
  // Add RenderViewSettings
  if (vtkSMProxy* renproxy = server->proxyManager()->GetProxy("settings", "RenderViewSettings"))
  {
    proxies_to_show.push_back(renproxy);
  }

  if (vtkSMProxy* renproxy =
        server->proxyManager()->GetProxy("settings", "RenderViewInteractionSettings"))
  {
    proxies_to_show.push_back(renproxy);
  }

  // Add color palette.
  if (vtkSMProxy* proxy = server->proxyManager()->GetProxy("global_properties", "ColorPalette"))
  {
    proxies_to_show.push_back(proxy);
  }

  vtkSMSettings* settings = vtkSMSettings::GetInstance();
  const char* systemdefaults = "{\"settings\" : { \"RenderViewInteractionSettings\" : {\
          \"Camera2DManipulators\" : [ 1, 3, 2, 0, 2, 6, 0, 1, 4 ],\
         \"Camera3DManipulators\" : [ 4, 2, 1, 0, 4, 1, 0, 4, 6 ]\
      },\
      \"RenderViewSettings\" : {\
         \"LODThreshold\" : 1024}}}";
  settings->AddCollectionFromString(systemdefaults, 2.0);
  foreach (vtkSMProxy* proxy, proxies_to_show)
  {
    if (!settings->GetProxySettings(proxy))
      proxy->ResetPropertiesToXMLDefaults();

    QString proxyName = proxy->GetXMLName();

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(proxy->GetXMLLabel());
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* container = new QWidget(scrollArea);
    container->setObjectName("Container");
    container->setContentsMargins(6, 0, 6, 0);

    QVBoxLayout* vbox = new QVBoxLayout(container);
    vbox->setMargin(0);
    vbox->setSpacing(0);

    pqProxyWidget* widget = new pqProxyWidget(proxy, container);
    widget->setObjectName("ProxyWidget");
    widget->setApplyChangesImmediately(false);
    widget->setView(NULL);

    //    widget->connect(this, SIGNAL(accepted()), SLOT(apply()));
    //    widget->connect(this, SIGNAL(rejected()), SLOT(reset()));
    //    this->connect(widget, SIGNAL(restartRequired()), SLOT(showRestartRequiredMessage()));
    vbox->addWidget(widget);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    vbox->addItem(spacer);

    scrollArea->setWidget(container);
    // show panel widgets, including advanced
    widget->filterWidgets(true);

    this->Internal->stackedWidget->addWidget(scrollArea);

    this->connect(widget, SIGNAL(changeAvailable()), SIGNAL(changesAvailable()));
    //    widget->connect(this, SIGNAL(filterWidgets(bool, QString)), SLOT(filterWidgets(bool, QString)));
    this->Internal->ProxyWidgets.push_back(widget);
  }

  QIntValidator* validator = new QIntValidator(this->Internal->maxNumberCloudPoints);
  this->Internal->maxNumberCloudPoints->setValidator(validator);
  int currentMax = this->maxNumberOfCloudPoints();
  this->Internal->maxNumberCloudPoints->setText(QString::number(currentMax));

  // start fresh
  this->loadGlobalPropertiesFromSettings();

  QObject::connect(this->Internal->maxNumberCloudPoints, SIGNAL(textChanged(const QString&)), this,
    SIGNAL(changesAvailable()));

  QObject::connect(this->Internal->maxNumberCloudPoints, SIGNAL(editingFinished()), this,
    SIGNAL(defaultMaxNumberOfPointsChanged()));

  QObject::connect(
    this->Internal->dirMeshBrowserButton, SIGNAL(clicked()), this, SLOT(chooseMeshStorageDir()));
  QObject::connect(
    this->Internal->dirTempBrowserButton, SIGNAL(clicked()), this, SLOT(chooseTempScratchDir()));
}

qtCMBApplicationOptions::~qtCMBApplicationOptions()
{
  delete this->Internal;
}

void qtCMBApplicationOptions::setPage(const QString& page)
{
  int count = this->Internal->stackedWidget->count();
  for (int i = 0; i < count; i++)
  {
    if (this->Internal->stackedWidget->widget(i)->objectName() == page)
    {
      this->Internal->stackedWidget->setCurrentIndex(i);
      break;
    }
  }
}

QStringList qtCMBApplicationOptions::getPageList()
{
  QStringList pages;

  int count = this->Internal->stackedWidget->count();
  for (int i = 0; i < count; i++)
  {
    pages << this->Internal->stackedWidget->widget(i)->objectName();
  }
  return pages;
}

void qtCMBApplicationOptions::applyChanges()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  int currentMax = this->maxNumberOfCloudPoints();
  int newMax = this->Internal->maxNumberCloudPoints->text().toInt();
  if (currentMax != newMax)
  {
    cmbSettings->setValue("MaxNumberOfCloudPoints", newMax);
    emit defaultMaxNumberOfPointsChanged();
  }
  // temp dirs
  cmbSettings->setValue("DefaultTempScratchDirectory", this->Internal->dirTempScratchFiles->text());
  cmbSettings->setValue("DefaultMeshStorageDirectory", this->Internal->dirStoreMeshes->text());

  this->loadGlobalPropertiesFromSettings();
  this->saveOptions();

  // render all views.
  pqApplicationCore::instance()->render();
}

void qtCMBApplicationOptions::loadGlobalPropertiesFromSettings()
{
  foreach (pqProxyWidget* pw, this->Internal->ProxyWidgets)
    pw->apply();
}

void qtCMBApplicationOptions::loadBuiltinColorPresets()
{
  vtkNew<vtkSMTransferFunctionPresets> presets;
  // See if we need to add the elivation presets
  unsigned int num = presets->GetNumberOfPresets();
  bool foundEli1 = false, foundEli2 = false;
  for (unsigned int i = 0; i < num; i++)
  {
    if (presets->GetPresetName(i) == "CMB Elevation Map 1")
    {
      foundEli1 = true;
    }
    else if (presets->GetPresetName(i) == "CMB Elevation Map 2")
    {
      foundEli2 = true;
    }
  }
  if (!foundEli1)
  {
    std::string preset =
      "{ \"ColorSpace\" : \"RGB\", \"NanColor\" : [ 1, 1, 0 ], \"Discretize\" : 0,\
\"RGBPoints\" : [\
-1000.0, 0.0, 0.0, 0.498,\
 -100.0, 0.0, 0.0, 1.0,\
    0.0, 0.0, 1.0, 1.0,\
    1.0, 0.333, 1.0, 0.0,\
    2.0, 0.1216, 0.3725, 0.0,\
  250.0, 1.0, 1.0, 0.0,\
  750.0, 1.0, 0.333, 0.0]}";
    presets->AddPreset("CMB Elevation Map 1", preset);
  }
  if (!foundEli2)
  {
    std::string preset =
      "{ \"ColorSpace\" : \"RGB\", \"NanColor\" : [ 1, 1, 0 ], \"Discretize\" : 0,\
\"RGBPoints\" : [\
-5000.0, 0.0, 0.0, 0.0,\
-1000.0, 0.0, 0.0, 1.0,\
 -100.0, 0.129412, 0.345098, 0.996078,\
  -50.0, 0.0, 0.501961, 1.0,\
  -10.0, 0.356863, 0.678431, 1.0,\
   -0.0, 0.666667, 1.0, 1.0,\
    0.01, 0.0, 0.250998, 0.0,\
   10.0, 0.301961, 0.482353, 0.0,\
   25.0, 0.501961, 1.0, 0.501961,\
  500.0, 0.188224, 1.0, 0.705882,\
 1000.0, 1.0, 1.0, 0.0,\
 2500.0, 0.505882, 0.211765, 0.0,\
 3200.0, 0.752941, 0.752941, 0.752941,\
 6000.0, 1.0, 1.0, 1.0]}";
    presets->AddPreset("CMB Elevation Map 2", preset);
  }
}

void qtCMBApplicationOptions::resetChanges()
{
  int maxNumOfPtsCloud = this->maxNumberOfCloudPoints();
  this->Internal->maxNumberCloudPoints->setText(QString::number(maxNumOfPtsCloud));

  this->Internal->dirStoreMeshes->setText(this->defaultMeshStorageDirectory().c_str());
  this->Internal->dirTempScratchFiles->setText(this->defaultTempScratchDirectory().c_str());
  foreach (pqProxyWidget* pw, this->Internal->ProxyWidgets)
    pw->reset();
}

void qtCMBApplicationOptions::saveOptions()
{
  // save the settings
  vtkSMSettings* settings = vtkSMSettings::GetInstance();
  foreach (pqProxyWidget* pw, this->Internal->ProxyWidgets)
  {
    settings->SetProxySettings(pw->proxy());
  }
}

void qtCMBApplicationOptions::restoreDefaults()
{
  foreach (pqProxyWidget* pw, this->Internal->ProxyWidgets)
  {
    pw->proxy()->ResetPropertiesToXMLDefaults();
  }
}

pqSettings* qtCMBApplicationOptions::cmbAppSettings()
{
  if (!this->Internal->CMBCommonSettings)
  {
    pqOptions* options =
      pqOptions::SafeDownCast(vtkProcessModule::GetProcessModule()->GetOptions());
    if (options && options->GetDisableRegistry())
    {
      QTemporaryFile tFile;
      tFile.open();
      this->Internal->CMBCommonSettings = new pqSettings(tFile.fileName() + ".ini", true, this);
      this->Internal->CMBCommonSettings->clear();
    }
    else
    {
      this->Internal->CMBCommonSettings = new pqSettings(QApplication::organizationName(),
        QString("CmbAppsCommon") + QApplication::applicationVersion(), this);
    }
  }
  return this->Internal->CMBCommonSettings;
}

int qtCMBApplicationOptions::maxNumberOfCloudPoints()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  // default to 0.1M points
  return cmbSettings->value("MaxNumberOfCloudPoints", 100000).toInt();
}

std::string qtCMBApplicationOptions::defaultMeshStorageDirectory()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  return cmbSettings->value("DefaultMeshStorageDirectory", QDir::tempPath())
    .toString()
    .toStdString();
}

std::string qtCMBApplicationOptions::defaultTempScratchDirectory()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  return cmbSettings->value("DefaultTempScratchDirectory", QDir::tempPath())
    .toString()
    .toStdString();
}

void qtCMBApplicationOptions::chooseMeshStorageDir()
{
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(), this,
    "Select Mesh Storage Directory", this->defaultMeshStorageDirectory().c_str());
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
  {
    //update the line-edit with the new folder
    this->Internal->dirStoreMeshes->setText(dialog.getSelectedFiles()[0]);
    emit this->changesAvailable();
  }
}

void qtCMBApplicationOptions::chooseTempScratchDir()
{
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(), this,
    "Select Temp Scratch Directory", this->defaultTempScratchDirectory().c_str());
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
  {
    //update the line-edit with the new folder
    this->Internal->dirTempScratchFiles->setText(dialog.getSelectedFiles()[0]);
    emit this->changesAvailable();
  }
}
