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
#include <QObject>
#include <QMap>
#include <QList>
#include <QPointer>
#include <QGridLayout>
#include <QDialog>
#include <vector>
#include <fstream>
#include "qtCMBArcWidgetManager.h"
#include "cmbSystemConfig.h"
#include "pqGeneralTransferFunctionWidget.h"
#include "pqCMBSceneObjectBase.h"
#include "pqCMBModifierArc.h"

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

class CMBAPPCOMMON_EXPORT pqCMBModifierArcManager : public QObject
{
  Q_OBJECT

public:
  pqCMBModifierArcManager(QLayout *layout,
                          pqServer *server,
                          pqRenderView *renderer);
  virtual ~pqCMBModifierArcManager();

  QTableWidget *getWidget() const { return this->TableWidget;}
  qtCMBArcWidgetManager* arcWidgetManager() const {return this->ArcWidgetManager;}

  void AddLinePiece(pqCMBModifierArc *dataObj, int visible=1);
  void AddFunction(cmbProfileFunction * fun);

  std::vector<int> getCurrentOrder(QString, int pindx);

  bool remove(int i, bool all = true);

  void setSourceType(pqCMBSceneObjectBase::enumObjectType st)
  { this->SourceType = st; }

  pqCMBSceneObjectBase::enumObjectType getSourceType() const
  { return this->SourceType; }

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
  void addProxy(QString, int pid, pqPipelineSource*);
  void clearProxies();
  void addLine();
  void selectLine(int lid);
  void update();
  void removeArc();
  void doneModifyingArc();
  void sendOrder();
  void enableAbsolute();
  void disableAbsolute();
  void onSaveProfile();
  void onLoadProfile();
  void onSaveArc();
  void onLoadArc();

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
  QTableWidget * TableWidget;
  std::vector<pqCMBModifierArc*> ArcLines;
  pqCMBModifierArc * CurrentModifierArc;
  QMap< QString, QMap<int, vtkSMSourceProxy*> > ServerProxies;
  QMap< QString, QMap<int, int> > ServerRows;
  std::vector< QMap< QString, QMap<int, bool> > > ArcLinesApply;
  qtCMBArcWidgetManager* ArcWidgetManager;
  QGridLayout* functionLayout;
  qtCMBManualFunctionWidget* ManualFunctionWidget;
  qtCMBProfileWedgeFunctionWidget* WedgeFunctionWidget;
  cmbProfileFunction * selectedFunction;
  pqCMBSceneObjectBase::enumObjectType SourceType;
  pqRenderView * view;
  pqServer * server;
  bool addPointMode;

  void initialize();
  void setRow(int row, pqCMBModifierArc * line);

  Ui_qtArcFunctionControl * UI;
  Ui_qtModifierArcDialog * UI_Dialog;
  QDialog * Dialog;

  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;

  void addApplyControl();

  void setUpTable();

  void setUpPointsTable();

  void setDatasetTable(int inId);

  void addNewArc(pqCMBModifierArc* arc);
  void check_save();

  void selectFunction(cmbProfileFunction* fun);

  void addItemToTable(pqCMBModifierArc::pointFunctionWrapper const* mp, bool select = false);

  pqCMBModifierArcManagerInternal * Internal;

protected slots:
  void updateLineFunctions();
  void accepted();
  void nameChanged(QString);
  void setToDefault();
  void newFunction();
  void deleteFunction();
  void cloneFunction();
  void functionTypeChanged(int);
  void functionModeChanged(int);
  void pointDisplayed(int);
  void addPoint();
  void deletePoint();
  void addPoint(vtkIdType);

protected slots:
  void onLineChange(int Id);
};

#endif /* __pqCMBModifierArcManager_h */
