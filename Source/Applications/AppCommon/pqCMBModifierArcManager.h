//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModifierArcManager - .
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModifierArcManager_h
#define __pqCMBModifierArcManager_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqCMBModifierArc.h"
#include "pqCMBSceneObjectBase.h"
#include "pqGeneralTransferFunctionWidget.h"
#include "qtCMBArcWidgetManager.h"
#include <QDialog>
#include <QGridLayout>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <fstream>
#include <vector>
#include <vtkBoundingBox.h>

class QTableWidget;
class pqCMBModifierArc;
class pqDataRepresentation;
class pqPipelineSource;
class QTableWidgetItem;
class vtkSMSourceProxy;
class Ui_qtArcFunctionControl;
class Ui_qtModifierArcDialog;
class qtCMBArcWidgetManager;
class pqServer;
class pqRenderView;
class pqPipelineSource;
class qtCMBManualFunctionWidget;
class qtCMBManualProfilePointFunctionModifier;
class cmbProfileFunction;
class qtCMBProfileWedgeFunctionWidget;
class pqCMBModifierArcManagerInternal;
class vtkPointSelectedCallback;

class CMBAPPCOMMON_EXPORT pqCMBModifierArcManager : public QObject
{
  Q_OBJECT

public:
  friend class vtkPointSelectedCallback;
  pqCMBModifierArcManager(QLayout* layout, pqServer* server, pqRenderView* renderer);
  ~pqCMBModifierArcManager() override;

  QTableWidget* getWidget() const { return this->TableWidget; }
  qtCMBArcWidgetManager* arcWidgetManager() const { return this->ArcWidgetManager; }

  void AddLinePiece(pqCMBModifierArc* dataObj, int visible = 1);
  void AddFunction(cmbProfileFunction* fun);

  std::vector<int> getCurrentOrder(QString, int pindx);

  bool remove(int i, bool all = true);

  void setSourceType(pqCMBSceneObjectBase::enumObjectType st) { this->SourceType = st; }

  pqCMBSceneObjectBase::enumObjectType getSourceType() const { return this->SourceType; }

  void setUseNormal(bool un);

public slots:
  void showDialog();
  void clear();
  void onClearSelection();
  void onItemChanged(QTableWidgetItem*);
  void onDatasetChange(QTableWidgetItem*);
  void onCurrentObjectChanged();
  void unselectAllRows();
  void onSelectionChange();
  void onFunctionSelectionChange();
  void onPointsSelectionChange();
  void enableSelection();
  void disableSelection();
  void applyAgain();
  void addProxy(QString, int pid, vtkBoundingBox dataBounds, pqPipelineSource*);
  void clearProxies();
  void addLine();
  void selectLine(int lid);
  void update();
  void removeArc();
  void doneModifyingArc();
  void sendOrder();
  void enableAbsolute();
  void disableAbsolute();
  void onSaveArc();
  void onLoadArc();
  void importFunction();

signals:
  void currentObjectChanged(pqCMBModifierArc*);
  void selectionChanged(int);
  void requestRender();
  void removed(int i);
  void orderChanged();
  void functionsUpdated();
  void applyFunctions();
  void updateDisplacementSlineControl(double, double, double);
  void updateWeightSlineControl(double, double, double);
  void changeDisplacementFunctionType(bool);
  void changeWeightFunctionType(bool);

  void addingNewArc();
  void modifyingArcDone();

protected:
  QTableWidget* TableWidget;
  std::vector<pqCMBModifierArc*> ArcLines;
  pqCMBModifierArc* CurrentModifierArc;
  struct DataSource
  {
  public:
    DataSource(vtkBoundingBox b = vtkBoundingBox(), vtkSMSourceProxy* sm = NULL)
      : box(b)
      , source(sm)
    {
    }
    vtkBoundingBox box;
    vtkSMSourceProxy* source;
  };
  QMap<QString, QMap<int, DataSource> > ServerProxies;
  QMap<QString, QMap<int, int> > ServerRows;
  std::vector<QMap<QString, QMap<int, bool> > > ArcLinesApply;
  qtCMBArcWidgetManager* ArcWidgetManager;
  QGridLayout* functionLayout;
  qtCMBManualFunctionWidget* ManualFunctionWidget;
  qtCMBProfileWedgeFunctionWidget* WedgeFunctionWidget;
  cmbProfileFunction* selectedFunction;
  pqCMBSceneObjectBase::enumObjectType SourceType;
  pqRenderView* view;
  pqServer* server;
  bool addPointMode;

  bool useNormal;

  void initialize();
  void setRow(int row, pqCMBModifierArc* line);

  QDialog* Dialog;

  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;

  void addApplyControl();

  void setUpTable();

  void setUpPointsTable();

  void setDatasetTable(int inId);

  void addNewArc(pqCMBModifierArc* arc);
  void check_save();

  void selectFunction(cmbProfileFunction* fun);

  void updateUiControls();

  int addItemToTable(pqCMBModifierArc::pointFunctionWrapper const* mp, bool select = false);

  pqCMBModifierArcManagerInternal* Internal;

protected slots:
  void updateLineFunctions();
  void accepted();
  void nameChanged(QString);
  void setToDefault();
  void deleteFunction();
  void cloneFunction();
  void functionTypeChanged(int);
  void functionModeChanged(int);
  void pointDisplayed(int);
  void addPoint();
  void deletePoint();
  int addPoint(vtkIdType);
  void editArc();
  void computeChange();

protected slots:
  void onLineChange(int Id);
};

#endif /* __pqCMBModifierArcManager_h */
