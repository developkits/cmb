//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelManager -
// .SECTION Description

#ifndef __qtModelManager_h
#define __qtModelManager_h

#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/model/StringData.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <QColor>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <set>
#include <vector>

namespace smtk
{
namespace attribute
{
class qtMeshSelectionItem;
}
namespace io
{
class Logger;
}
}

class vtkPVSMTKModelInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqOutputPort;
class pqPipelineSource;
class pqRenderView;
class pqServer;
class pqSMTKModelInfo;
class pqSMTKMeshInfo;
class smtkAuxGeoInfo;

class pqCMBModelManager : public QObject
{
  Q_OBJECT

public:
  pqCMBModelManager(pqServer*);
  ~pqCMBModelManager() override;
  vtkSMModelManagerProxy* managerProxy();
  smtk::model::StringData fileModelSessions(const std::string& filename);
  std::set<std::string> supportedFileTypes(const std::string& bridgeName = std::string());

  void supportedColorByModes(QStringList& types);
  void updateEntityColorTable(pqDataRepresentation* rep,
    const QMap<smtk::model::EntityRef, QColor>& colorEntities, const QString& colorByMode);
  void updateAttributeColorTable(pqDataRepresentation* rep, vtkSMProxy* lutProxy,
    const QMap<std::string, QColor>& colorAtts, const std::vector<std::string>& annList);
  void colorRepresentationByAttribute(pqDataRepresentation* rep,
    smtk::attribute::CollectionPtr attCollection, const QString& attDef, const QString& attItem);

  pqSMTKModelInfo* modelInfo(const smtk::model::EntityRef& entity);
  pqSMTKModelInfo* modelInfo(pqDataRepresentation* rep);
  smtkAuxGeoInfo* auxGeoInfo(const std::string& auxurl);
  QList<pqSMTKModelInfo*> selectedModels() const;
  QList<pqSMTKModelInfo*> allModels() const;
  smtk::model::EntityRef entityOfRepresentation(const pqDataRepresentation* rep);
  pqDataRepresentation* representationOfEntity(const smtk::model::EntityRef& ent);

  int numberOfModels();
  int numberOfRemoteSessions();

  QList<pqDataRepresentation*> modelRepresentations() const;

  pqSMTKModelInfo* activateModelRepresentation();
  void setActiveModelRepresentation(pqDataRepresentation*);

  bool DetermineFileReader(const std::string& filename, std::string& bridgeType,
    std::string& engineType, const smtk::model::StringData& bridgeTypes);
  pqServer* server();
  /// Update the representation for the model, returning true if the model was previously empty but now has renderable entities.
  bool updateModelRepresentation(const smtk::model::EntityRef& model);
  bool updateModelRepresentation(pqSMTKModelInfo* minfo);
  void updateModelMeshRepresentations(const smtk::model::Model& model);

  pqSMTKMeshInfo* meshInfo(const smtk::mesh::MeshSet& mesh);
  pqSMTKMeshInfo* meshInfo(pqDataRepresentation* rep);
  QList<pqSMTKMeshInfo*> selectedMeshes() const;
  QList<pqDataRepresentation*> meshRepresentations() const;
  QList<pqSMTKMeshInfo*> allMeshes() const;

  QList<smtkAuxGeoInfo*> selectedAuxGeos() const;
  smtk::common::UUIDs auxGeoRelatedEntities(const std::string& auxurl) const;

  /// A convenience method to sort models alphabetically and return the first one
  smtk::model::Model sortExistingModels(smtk::model::Models& models);

  /// Resets the active-view's camera the first time this method is called and not again until allowActiveCameraReset().
  bool resetActiveCameraIfAllowable();
  /// Make the next (and only the next) call to resetActiveCameraIfAllowable() change the camera.
  void allowActiveCameraReset();
signals:
  void newModelManagerProxy(
    vtkSMModelManagerProxy*); // Emitted each time a new model manager is created on the server.
  void currentModelCleared();
  void newSessionLoaded(const QStringList& bridgeNames);
  void sessionClosing(const smtk::model::SessionRef& sref);
  void newFileTypesAdded(const QStringList& fileTypes);
  void operationFinished(const smtk::model::OperatorResult&, const smtk::model::SessionRef& sref,
    bool hasNewModels, bool bModelGeometryChanged, bool hasNewMeshes);
  void requestMeshSelectionUpdate(const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*);
  void operationLog(const smtk::io::Logger& summary);
  void modelRepresentationAdded(pqDataRepresentation*);
  void newModelWithDimension(int);
  void newAuxiliaryGeometryWithDimension(int);
  void sessionIsNowEmpty(const smtk::model::SessionRef&);
  void newModelCreated(const smtk::model::EntityRef& newModel);
  void newModelsCreationFinished();
  void fileOpenedSuccessfully(const std::string& url);
  void modelRepresentationUpdated();

public slots:
  void clear();
  bool startNewSession(const std::string& bridgeName, const bool& createDefaultModel = true,
    const bool& useExistingSession = false);
  bool closeSession(const smtk::model::SessionRef& sref);
  void clearModelSelections();
  void clearMeshSelections();
  void clearAuxGeoSelections();
  smtk::model::OperatorPtr createFileOperator(const std::string& filename);
  bool startOperation(const smtk::model::OperatorPtr&);
  bool handleOperationResult(const smtk::model::OperatorResult& result,
    const smtk::common::UUID& bridgeSessionId, bool& hadNewModels, bool& bModelGeometryChanged,
    bool& hasNewMeshes, std::set<smtk::model::SessionRef>& emptySessions);
  void setActiveModelSource(const smtk::common::UUID&);

protected slots:
  void onPluginLoaded();

protected:
  void initialize();
  void initOperator(smtk::model::OperatorPtr brOp);

private:
  class qInternal;
  qInternal* Internal;
};

#endif
