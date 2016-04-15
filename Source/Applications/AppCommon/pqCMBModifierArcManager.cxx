//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBModifierArcManager.h"

#include "pqCMBLIDARPieceObject.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqCMBArc.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QDialog>
#include <QHeaderView>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPointer>

#include <fstream>
#include <sstream>

#include "pqSMAdaptor.h"
#include "pqCMBModifierArc.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "vtkSMRepresentationProxy.h"
#include <vtksys/SystemTools.hxx>
#include "ui_qtArcFunctionControl.h"
#include "ui_qtModifierArcDialog.h"
#include "vtkPVArcInfo.h"
#include "qtCMBArcWidget.h"
#include "pq3DWidgetFactory.h"
#include "vtkSMRenderViewProxy.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMVectorProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkNew.h"

#include "qtCMBManualFunctionWidget.h"
#include "qtCMBProfileWedgeFunctionWidget.h"
#include "cmbManualProfileFunction.h"
#include "cmbProfileFunction.h"
#include "cmbProfileWedgeFunction.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

#include "vtkCMBArcEditClientOperator.h"

#include "pqRepresentationHelperFunctions.h"

#include <vtkPoints.h>
#include <vtkActor.h> 
#include <vtkPolyDataMapper.h>
#include <vtkGlyph3D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <QVTKWidget.h>

#include "vtkSMCMBGlyphPointSourceProxy.h"

// enum for different column types
enum DataTableCol
{
  VisibilityCol=0,
  Id
};

class pqCMBModifierArcManagerInternal
{
public:
  QPointer<pqDataRepresentation> ArcPointSelectRepresentation;
  QPointer<pqPipelineSource> ArcPointSelectSource;
  vtkSMNewWidgetRepresentationProxy * editableWidget;
  //qtCMBArcWidget* PointSelectionWidget;
  QPointer<pqPipelineSource> SphereSource;
  QPointer<pqDataRepresentation> Representation;
  QPointer<pqPipelineSource> LineGlyphFilter;
};

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::pqCMBModifierArcManager(QLayout *layout,
                                                 pqServer *server,
                                                 pqRenderView *renderer)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->view = renderer;
  this->server = server;
  this->Internal = new pqCMBModifierArcManagerInternal;
  //this->Internal->PointSelectionWidget = NULL;
  this->Internal->editableWidget = NULL;

  this->Internal->SphereSource = builder->createSource("sources", "SphereSource", server);
  this->Internal->LineGlyphFilter = builder->createSource("filters", "ArcPointGlyphingFilter", server);
  this->Internal->Representation =
      builder->createDataRepresentation( this->Internal->LineGlyphFilter->getOutputPort(0),
                                         this->view,
                                         "GeometryRepresentation");

  //vtkSMSourceProxy* source = this->Internal->SphereSource->getProxy();
  pqSMAdaptor::setElementProperty(this->Internal->SphereSource->getProxy()->GetProperty("SetRadius"),1000);

  this->Internal->SphereSource->getProxy()->MarkModified(this->Internal->SphereSource->getProxy());

  vtkSMProxy *repProxy = this->Internal->Representation->getProxy();
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY( repProxy, "Color", vtkDataObject::POINT);

  pqSMAdaptor::setEnumerationProperty(repProxy->GetProperty("Representation"), "3D Glyphs");
  //vtkSMPropertyHelper(repProxy, "ImmediateModeRendering").Set(0);
  vtkSMPropertyHelper(repProxy, "GlyphType").Set(this->Internal->SphereSource->getProxy());
  vtkSMPropertyHelper(repProxy, "Scaling").Set(true);
  vtkSMPropertyHelper(repProxy, "ScaleMode").Set(2);
  vtkSMPropertyHelper(repProxy, "Orient").Set(true);
  vtkSMPropertyHelper(repProxy, "OrientationMode").Set(1);
  vtkSMPropertyHelper(repProxy, "MapScalars").Set(0);
  vtkSMPropertyHelper(repProxy, "SelectScaleArray").Set("Scaling");
  vtkSMPropertyHelper(repProxy, "SelectOrientationVectors").Set("Orientation");

  // This is a work arround for a bug in ParaView
  vtkSMPropertyHelper(repProxy, "ScaleFactor").Set(1.0);

  repProxy->UpdateVTKObjects();

  this->CurrentModifierArc = NULL;
  this->ManualFunctionWidget = NULL;
  this->WedgeFunctionWidget = NULL;
  this->selectedFunction = NULL;
  this->UI = new Ui_qtArcFunctionControl();
  QWidget * w = new QWidget();
  this->UI->setupUi(w);
  if(layout != NULL)
  {
    layout->addWidget(w);
    this->Dialog = NULL;
    this->UI_Dialog = NULL;
  }
  else
  {
    this->Dialog = new QDialog();
    this->UI_Dialog = new Ui_qtModifierArcDialog();
    this->UI_Dialog->setupUi(this->Dialog);
    this->UI_Dialog->modifierLayout->addWidget(w);
    QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(accepted()));
    //connect(this->Dialog, SIGNAL(rejected()), this, SLOT(clear()));
  }
  functionLayout = new QGridLayout(this->UI->functionControlArea);
  functionLayout->setMargin(0);

  this->TableWidget = this->UI->ModifyArcTable;
  this->clear();
  this->initialize();
  QObject::connect(this->UI->addLineButton, SIGNAL(clicked()),
                   this, SLOT(addLine()));
  QObject::connect(this->UI->removeLineButton, SIGNAL(clicked()),
                   this, SLOT(removeArc()));
  QObject::connect(this->UI->buttonUpdateLine, SIGNAL(clicked()),
                   this, SLOT(update()));
  QObject::connect(this->UI->ApplyAgain, SIGNAL(clicked()),
                   this, SLOT(applyAgain()));

  QObject::connect(this->UI->NewFunction, SIGNAL(clicked()),
                   this, SLOT(newFunction()));

  QObject::connect(this->UI->DeleteFunction, SIGNAL(clicked()),
                   this, SLOT(deleteFunction()));
  QObject::connect(this->UI->CloneFunction, SIGNAL(clicked()),
                   this, SLOT(cloneFunction()));

  QObject::connect(this->UI->FunctionName, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onFunctionSelectionChange()));
  QObject::connect(this->UI->FunctionType, SIGNAL(currentIndexChanged(int)),
                   this,  SLOT(functionTypeChanged(int)));
  QObject::connect(this->UI->PointMode, SIGNAL(currentIndexChanged(int)),
                   this,  SLOT(functionModeChanged(int)));

  QObject::connect(this->UI->EndPointDisplayed,SIGNAL(currentIndexChanged(int)),
                   this, SLOT(pointDisplayed(int)));
    

  QObject::connect(this->UI->Save, SIGNAL(clicked()), this, SLOT(onSaveProfile()));
  QObject::connect(this->UI->Load, SIGNAL(clicked()), this, SLOT(onLoadProfile()));

  QObject::connect(this->UI->FunctionName, SIGNAL(editTextChanged(QString const&)),
                   this, SLOT(nameChanged(QString)));

  QObject::connect(this->UI->SaveFunctions, SIGNAL(clicked()),
                   this, SLOT(onSaveArc()));
  QObject::connect(this->UI->LoadFunctions, SIGNAL(clicked()),
                   this, SLOT(onLoadArc()));

  this->ArcWidgetManager = new qtCMBArcWidgetManager(server, renderer);
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcSplit2(pqCMBArc*, QList<vtkIdType>)),
                   this, SLOT(doneModifyingArc()));
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcModified2(pqCMBArc*)),
                   this, SLOT(doneModifyingArc()));
  QObject::connect(this->ArcWidgetManager, SIGNAL(Finish()),
                   this, SLOT(doneModifyingArc()));
  QObject::connect( this->ArcWidgetManager, SIGNAL(selectedId(vtkIdType)),
                    this, SLOT(addPoint(vtkIdType)));
  this->UI->ArcControlTabs->setTabEnabled(1, false);
  this->UI->ArcControlTabs->setTabEnabled(2, false);
  this->UI->ArcControlTabs->setTabEnabled(3, false);
  QObject::connect(this, SIGNAL(selectionChanged(int)), this, SLOT(selectLine(int)));
  QObject::connect(this, SIGNAL(orderChanged()), this, SLOT(sendOrder()));
  this->check_save();
  this->addPointMode = false;
}

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::~pqCMBModifierArcManager()
{
  if(this->Internal->ArcPointSelectSource)
  {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(this->Internal->ArcPointSelectSource);
    this->Internal->ArcPointSelectSource = NULL;
  }
  delete this->Internal;
  this->UI->ArcControlArea->takeWidget();
  this->clear();
  delete this->Dialog;
  delete this->UI;
  delete this->ArcWidgetManager;
  //delete this->Internal->PointSelectionWidget;
  //TODO clean up editableWidget
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::initialize()
{
  this->TableWidget->setColumnCount(2);
  this->TableWidget->setHorizontalHeaderLabels(QStringList() << tr("Apply")
                                                             << tr("Path ID"));

  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->TableWidget->verticalHeader()->hide();

  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onCurrentObjectChanged()), Qt::QueuedConnection);
  QObject::connect(this->TableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onItemChanged(QTableWidgetItem*)));
  QObject::connect(this->UI->DatasetTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onDatasetChange(QTableWidgetItem*)));
  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onSelectionChange()));

  this->UI->points->setColumnCount(5);
  this->UI->points->setHorizontalHeaderLabels( QStringList() << tr("Point Index") << tr("Point Id")
                                                             << tr("X") << tr("Y")
                                                             << tr("Function"));
  this->UI->points->setSelectionMode(QAbstractItemView::SingleSelection);
  this->UI->points->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->UI->points->verticalHeader()->hide();

  QObject::connect(this->UI->points, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onPointsSelectionChange()));
  QObject::connect(this->UI->AddPoint, SIGNAL(clicked()), this, SLOT(addPoint()));
  QObject::connect(this->UI->RemovePoint, SIGNAL(clicked()), this, SLOT(deletePoint()));

}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::accepted()
{
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
      {
        if(this->ArcLines[i]!= NULL)
        {
          this->ArcLines[i]->updateArc(source);
        }
      }
    }
  }
  this->sendOrder();
  //this->clear();
  if(this->Dialog != NULL) this->Dialog->hide();
  emit(applyFunctions());
}

void pqCMBModifierArcManager::showDialog()
{
  if(this->Dialog != NULL)
  {
    this->Dialog->show();
  }
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
      {
        if(this->ArcLines[i]!= NULL)
        {
          QList< QVariant > v;
          v << this->ArcLines[i]->getId();
          pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
          v.clear();
          v << this->ArcLines[i]->getId() << 1;
          pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
          source->UpdateVTKObjects();
        }
      }
    }
  }
  addApplyControl();
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::clear()
{
  selectLine(-1);
  this->CurrentModifierArc = NULL;
  delete this->ManualFunctionWidget;
  delete this->WedgeFunctionWidget;
  this->ManualFunctionWidget = NULL;
  this->selectedFunction = NULL;
  this->TableWidget->clearContents();
  this->TableWidget->setRowCount(0);
  this->UI->points->clearContents();
  this->UI->points->setRowCount(0);
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
    {
    delete ArcLines[i];
    }

  ArcLines.clear();
  ServerProxies.clear();
  ServerRows.clear();
  ArcLinesApply.clear();
}

void pqCMBModifierArcManager::enableSelection()
{
  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void pqCMBModifierArcManager::disableSelection()
{
  this->TableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onClearSelection()
{
  this->TableWidget->blockSignals(true);
  this->TableWidget->clearSelection();
  //TODO clear the selected values form system
  this->TableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::AddLinePiece(pqCMBModifierArc *dataObj, int visible)
{
  this->TableWidget->insertRow(this->TableWidget->rowCount());
  int row =  this->TableWidget->rowCount()-1;

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  setRow(row, dataObj);

  QTableWidgetItem* objItem = new QTableWidgetItem();
  QVariant vdata;
  vdata.setValue(static_cast<void*>(dataObj));
  objItem->setData(Qt::UserRole, vdata);

  this->TableWidget->setItem(row, VisibilityCol, objItem);
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);

  this->TableWidget->resizeColumnsToContents();
  connect(dataObj, SIGNAL(functionChanged(int)),
          this, SLOT(onLineChange(int)));
  unselectAllRows();
  this->TableWidget->setRangeSelected(QTableWidgetSelectionRange(row,0,row,Id), true);
  if(visible)
    emit orderChanged();
}

void pqCMBModifierArcManager::AddFunction(cmbProfileFunction * fun)
{
  QVariant vdata;
  vdata.setValue(static_cast<void*>(fun));
  this->UI->FunctionName->addItem(fun->getName().c_str(), vdata);
  this->UI->DeleteFunction->setEnabled(this->UI->FunctionName->count()>1);
}

void pqCMBModifierArcManager::unselectAllRows()
{
  this->TableWidget->setRangeSelected(
                      QTableWidgetSelectionRange(0,0,
                                                 this->TableWidget->rowCount()-1,
                                                 Id),
                      false);
}

void pqCMBModifierArcManager::setRow(int row, pqCMBModifierArc * dataObj)
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QString s = QString::number(dataObj->getId());
  QTableWidgetItem* v = new QTableWidgetItem(s);
  this->TableWidget->setItem(row, Id, v);
  v->setFlags(commFlags);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onCurrentObjectChanged()
{
}

void pqCMBModifierArcManager::onLineChange(int id)
{
  pqCMBModifierArc * line = NULL;
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
    {
    if(ArcLines[i] != NULL && ArcLines[i]->getId() == id)
      {
      line = ArcLines[i];
      }
    }
  if(line == NULL) return;
  for( int i = 0; i < this->TableWidget->rowCount(); ++i)
    {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    if(tmp->text().toInt() == id)
      {
      this->setRow(i, line);
      }
    }
}

bool pqCMBModifierArcManager::remove(int id, bool all)
{
  this->TableWidget->blockSignals(true);

  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  int row = selected.front().topRow();
  int sid = this->TableWidget->item( row, Id )->text().toInt();

  if(id == -1)//Just remove the selected one
    {
    id = sid;
    }
  if(id == sid)
    {
    this->TableWidget->removeRow(row);
    }
  if(all)
    {
    int r = this->TableWidget->rowCount() - 1;
    for(; r >= 0; --r)
      {
      QTableWidgetItem *        tmp = this->TableWidget->item( r, Id );
      if(tmp->text().toInt() == id)
        {
        this->TableWidget->removeRow(r);
        }
      }
    }
  else
    {
    for(int i = 0; i < this->TableWidget->rowCount(); ++i)
      {
      QTableWidgetItem *        tmp = this->TableWidget->item( i, Id );
      if(id == tmp->text().toInt())
        {
        this->TableWidget->blockSignals(false);
        emit(orderChanged());
        onClearSelection();
        return true;
        }
      }
    }
  ArcLines[id] = NULL;
  this->TableWidget->blockSignals(false);
  emit(orderChanged());
  onClearSelection();
  this->check_save();
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onItemChanged( QTableWidgetItem* item)
{
  int id = this->TableWidget->item( item->row(), Id )->text().toInt();
  pqCMBModifierArc* dl = ArcLines[id];
  if(dl == NULL) return;
  if(item->column() == VisibilityCol)
    {
    emit(orderChanged());
    }
}

void pqCMBModifierArcManager::onDatasetChange(QTableWidgetItem* item)
{
  if(this->CurrentModifierArc == NULL) return;
  int id = this->CurrentModifierArc->getId();
  if(item->column() == 0)
    {
    QString fname = this->UI->DatasetTable->item( item->row(), 1 )->text();
    int pieceIdx = this->UI->DatasetTable->item( item->row(), 2 )->text().toInt();
    bool visible = item->checkState() == Qt::Checked;
    ArcLinesApply[id][fname][pieceIdx] = visible;
    emit orderChanged();
    }
}

void pqCMBModifierArcManager::applyAgain()
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  this->TableWidget->blockSignals(true);
  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  if (!selected.empty())
    {
    int row = selected.front().topRow();
    int id = this->TableWidget->item( row, Id )->text().toInt();
    bool visible = this->TableWidget->item( row, VisibilityCol )->checkState() == Qt::Checked;
    pqCMBModifierArc* dl = ArcLines[id];
    this->TableWidget->insertRow(this->TableWidget->rowCount());
    row =  this->TableWidget->rowCount()-1;

    setRow(row, dl);

    QTableWidgetItem* objItem = new QTableWidgetItem();
    QVariant vdata;
    vdata.setValue(static_cast<void*>(dl));
    objItem->setData(Qt::UserRole, vdata);

    this->TableWidget->setItem(row, VisibilityCol, objItem);
    objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
    objItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);

    this->TableWidget->resizeColumnsToContents();
    if(visible) emit orderChanged();
    }
  this->TableWidget->blockSignals(false);
}

void pqCMBModifierArcManager::onSelectionChange()
{
  this->TableWidget->blockSignals(true);
  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  if (!selected.empty())
    {
    int row = selected.front().topRow();
    int id = this->TableWidget->item( row, Id )->text().toInt();
    emit selectionChanged(id);
    }
  this->TableWidget->blockSignals(false);
}

void pqCMBModifierArcManager::onFunctionSelectionChange()
{
  cmbProfileFunction* fun = NULL;
  int	index = this->UI->FunctionName->currentIndex();
  if(index != -1)
  {
    QVariant d = this->UI->FunctionName->itemData(index);
    void * dv = d.value<void *>();
    fun = static_cast<cmbProfileFunction*>(dv);
  }
  selectFunction(fun);
}

void pqCMBModifierArcManager::onPointsSelectionChange()
{
  QList<QTableWidgetItem *>selected =	this->UI->points->selectedItems();
  if( !selected.empty() )
  {
    vtkSMSourceProxy * pointDisplaySource =
                        vtkSMSourceProxy::SafeDownCast(this->Internal->LineGlyphFilter->getProxy());
    QVariant qv = selected[0]->data(Qt::UserRole);
    pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
    this->UI->FunctionName->setCurrentIndex(this->UI->FunctionName->findText(wrapper->getName().c_str()));
    selectFunction(const_cast<cmbProfileFunction*>(wrapper->getFunction()));
    {
      double pt[3];
      vtkPVArcInfo* ai =  CurrentModifierArc->GetCmbArc()->getArcInfo();
      pointDisplaySource->InvokeCommand("clearVisible");
      pqSMAdaptor::setElementProperty(pointDisplaySource->GetProperty("setVisible"), wrapper->getPointIndex());
      pointDisplaySource->UpdateVTKObjects();
      pqSMAdaptor::setElementProperty(pointDisplaySource->GetProperty("setVisible"), -1);
      pointDisplaySource->UpdatePipeline();
      this->Internal->Representation->getProxy()->UpdateVTKObjects();
      emit requestRender();
    }
    //this->ArcWidgetManager->highlightPoint(wrapper->getPointIndex());
  }
}

std::vector<int> pqCMBModifierArcManager::getCurrentOrder(QString fname, int pindx)
{
  std::vector<int> result;
  for(int i = 0; i < this->TableWidget->rowCount(); ++i)
    {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    int id = tmp->text().toInt();
    tmp = this->TableWidget->item( i, VisibilityCol );
    if(tmp->checkState() == Qt::Checked && ArcLinesApply[id][fname][pindx])
      {
      result.push_back(id);
      }
    }
  return result;
}

void pqCMBModifierArcManager::addLine()
{
  emit this->addingNewArc();
  selectLine(-2);
}

void pqCMBModifierArcManager::selectLine(int sid)
{
  vtkSMSourceProxy * pointDisplaySource =
                        vtkSMSourceProxy::SafeDownCast(this->Internal->LineGlyphFilter->getProxy());
  if(this->CurrentModifierArc != NULL)
    {
    if(sid == this->CurrentModifierArc->getId()) return;
    pqSMAdaptor::setInputProperty(pointDisplaySource->GetProperty("Input"), NULL, 0);
    pointDisplaySource->MarkModified(pointDisplaySource);
    pointDisplaySource->UpdateVTKObjects();
    this->UI->ArcControlArea->takeWidget();
    this->CurrentModifierArc->switchToNotEditable();
    this->CurrentModifierArc = NULL;
    this->UI->removeLineButton->setEnabled(false);
    this->UI->buttonUpdateLine->setEnabled(false);
    this->UI->addLineButton->setEnabled(true);
    this->UI->ArcControlTabs->setTabEnabled(1, false);
    this->UI->ArcControlTabs->setTabEnabled(2, false);
    this->UI->ArcControlTabs->setTabEnabled(3, false);
    this->UI->ApplyAgain->setEnabled(false);
    this->UI->LoadFunctions->setEnabled(true);
    if(this->UI_Dialog != NULL)
      {
      QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(true);
      }
    }
  if(sid == -1 || sid < -2)
    {
    return;
    }
  else if(sid == -2) //create new one
    {
    pqCMBModifierArc * dig = new pqCMBModifierArc();
    this->ArcWidgetManager->setActiveArc(dig->GetCmbArc());
    this->ArcWidgetManager->create();
    QWidget * arc =this->ArcWidgetManager->getActiveWidget();
    this->UI->ArcControlArea->setWidget(arc);
    //TODO disable selection and add button
    arc->setMinimumHeight(300);
    arc->setVisible(true);
    QObject::connect(arc,SIGNAL(contourDone()),
                     this,SLOT(doneModifyingArc()));
    QObject::connect( dig, SIGNAL(requestRender()),
                     this, SIGNAL(requestRender()) );

    addNewArc(dig);

    this->disableSelection();
    this->UI->addLineButton->setEnabled(false);
    this->UI->SaveFunctions->setEnabled(false);
    this->UI->LoadFunctions->setEnabled(false);
    if(this->UI_Dialog != NULL)
      {
      QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(false);
      }
    }
  else
    {
    if(static_cast<size_t>(sid)<ArcLines.size() && ArcLines[sid] != NULL)
      {
      this->CurrentModifierArc = ArcLines[sid];
      this->CurrentModifierArc->switchToEditable();
      this->ArcWidgetManager->setActiveArc(this->CurrentModifierArc->GetCmbArc());
      this->ArcWidgetManager->edit();
      this->UI->ArcControlArea->setWidget(this->ArcWidgetManager->getActiveWidget());
      this->UI->removeLineButton->setEnabled(true);
      this->UI->buttonUpdateLine->setEnabled(true);
      this->UI->ApplyAgain->setEnabled(true);
      this->UI->LoadFunctions->setEnabled(true);
      }
    }
  if(this->CurrentModifierArc != NULL)
    {
    if(this->CurrentModifierArc->GetCmbArc()->getSource() != NULL)
    {
      pqSMAdaptor::setInputProperty(pointDisplaySource->GetProperty("Input"),
                                    this->CurrentModifierArc->GetCmbArc()->getSource()->getProxy(),
                                    0);
      pointDisplaySource->MarkModified(pointDisplaySource);
      pointDisplaySource->UpdateVTKObjects();
    }
    this->updateLineFunctions();
    this->setDatasetTable(sid);
    this->setUpPointsTable();
    this->UI->points->selectRow(0);

    this->UI->ArcControlTabs->setTabEnabled(1, true);
    this->UI->ArcControlTabs->setTabEnabled(2, true);
    this->UI->ArcControlTabs->setTabEnabled(3, true);
    }
}

void pqCMBModifierArcManager::addNewArc(pqCMBModifierArc* dig)
{
  this->CurrentModifierArc = dig;
  int sid = ArcLines.size();
  ArcLines.push_back(dig);
  addApplyControl();
  //TODO THIS NEEDS TO BE BETTER
  dig->setId(sid);
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      QList< QVariant > v;
      v <<sid;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
      v.clear();
      v <<sid << 1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
      source->UpdateVTKObjects();
    }
  }
  this->AddLinePiece(dig);
  this->check_save();
}

void pqCMBModifierArcManager::update()
{
  if(this->CurrentModifierArc == NULL)
    {
    return;
    }
  {//Update point functions
    pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
    vtkPVArcInfo* ai = arc->getArcInfo();
  }
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      this->CurrentModifierArc->updateArc(source);
      }
    }
  setUpPointsTable();
  emit(functionsUpdated());
  emit(requestRender());
}

void pqCMBModifierArcManager::removeArc()
{
  if(this->CurrentModifierArc == NULL)
    {
    return;
    }
  int id = this->CurrentModifierArc->getId();
  this->enableSelection();
  selectLine(-1);

  pqCMBModifierArc * tmp = ArcLines[id];
  bool hasMore = this->remove(id, false);
  if(!hasMore)
    {
    QObject::disconnect( tmp, SIGNAL(requestRender()),
                         this, SIGNAL(requestRender()) );
    ArcLines[id] = NULL;
    foreach(QString filename, ServerProxies.keys())
      {
      foreach(int pieceIdx, ServerProxies[filename].keys())
        {
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
        tmp->removeFromServer(source);
        }
      }

    delete tmp;
    }
  emit(requestRender());
}

void pqCMBModifierArcManager::addProxy(QString s, int pid, pqPipelineSource* ps)
{
  vtkSMSourceProxy* source = NULL;
  source = vtkSMSourceProxy::SafeDownCast(ps->getProxy() );
  ServerProxies[s].insert(pid, source);
  setUpTable();

  for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
  {
    if(this->ArcLines[i]!= NULL)
    {
      QList< QVariant > v;
      v << this->ArcLines[i]->getId();
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
      v.clear();
      v << this->ArcLines[i]->getId() << 1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
      source->UpdateVTKObjects();
      this->ArcLines[i]->updateArc(source);
      ArcLinesApply[i][s][pid] = true;
    }
  }

  this->sendOrder();
}

void pqCMBModifierArcManager::clearProxies()
{
  ServerProxies.clear();
  ServerRows.clear();
  ArcLinesApply.clear();
}

void pqCMBModifierArcManager::updateLineFunctions()
{
  if(this->CurrentModifierArc == NULL) return;
  pqCMBModifierArc * dig = this->CurrentModifierArc;
  std::vector<cmbProfileFunction *> funs;
  dig->getFunctions(funs);
  this->UI->FunctionName->blockSignals(true);
  this->UI->FunctionName->clear();
  for(unsigned int i = 0; i < funs.size(); ++i)
  {
    this->AddFunction(funs[i]);
  }
  this->UI->PointMode->setCurrentIndex(CurrentModifierArc->getFunctionMode());
  this->functionModeChanged(CurrentModifierArc->getFunctionMode());
  QString name;
  switch(CurrentModifierArc->getFunctionMode())
  {
    case pqCMBModifierArc::EndPoints:
    {
      if(this->UI->EndPointDisplayed->currentIndex() == 1)
      {
        name = dig->getEndFun()->getName().c_str();
        break;
      }
      //else falls through
    }
    case pqCMBModifierArc::Single:
      name = dig->getStartFun()->getName().c_str();
      break;
    case pqCMBModifierArc::PointAssignment:
    {
      QList<QTableWidgetItem *>selected =	this->UI->points->selectedItems();
      if( !selected.empty() )
      {
        QVariant qv = selected[0]->data(Qt::UserRole);
        pqCMBModifierArc::pointFunctionWrapper * wrapper =
          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
        name = wrapper->getName().c_str();
      }
      break;
    }
  }
  this->UI->FunctionName->setCurrentIndex(this->UI->FunctionName->findText(name));
  this->UI->FunctionName->blockSignals(false);
  onFunctionSelectionChange();
}

void pqCMBModifierArcManager::selectFunction(cmbProfileFunction* fun)
{
  delete ManualFunctionWidget;
  delete WedgeFunctionWidget;
  ManualFunctionWidget = NULL;
  WedgeFunctionWidget = NULL;
  selectedFunction = NULL;
  if(fun == NULL)
  {
    this->UI->CloneFunction->setEnabled(false);
    this->UI->DeleteFunction->setEnabled(false);
    this->UI->Save->setEnabled(false);
  }
  else
  {
    this->UI->CloneFunction->setEnabled(true);
    this->UI->DeleteFunction->setEnabled(true);
    this->UI->Save->setEnabled(true);
    this->UI->FunctionType->setCurrentIndex(static_cast<int>(fun->getType()));
    selectedFunction = fun;
    this->nameChanged(fun->getName().c_str());
    switch(CurrentModifierArc->getFunctionMode())
    {
      case pqCMBModifierArc::EndPoints:
        if(this->UI->EndPointDisplayed->currentIndex() == 1)
        {
          CurrentModifierArc->setEndFun(selectedFunction->getName());
          break;
        }
      case pqCMBModifierArc::Single:
        CurrentModifierArc->setStartFun(selectedFunction->getName());
        break;
      case pqCMBModifierArc::PointAssignment:
      {
        QList<QTableWidgetItem *>selected =	this->UI->points->selectedItems();
        if( !selected.empty() )
        {
          QVariant qv = selected[0]->data(Qt::UserRole);
          pqCMBModifierArc::pointFunctionWrapper * wrapper =
                         static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
          CurrentModifierArc->addFunctionAtPoint(wrapper->getPointId(),selectedFunction);
        }
        break;
      }
    }
    //bool isDefault = CurrentModifierArc->getDefaultFun() == fun;
    //this->UI->isDefault->setChecked(isDefault);
    //this->UI->isDefault->setEnabled(!isDefault);
    //this->UI->DeleteFunction->setEnabled(!isDefault); //TODO: FIX This
    switch(fun->getType())
    {
      case cmbProfileFunction::MANUAL:
        ManualFunctionWidget =
            new qtCMBManualFunctionWidget(dynamic_cast<cmbManualProfileFunction*>(fun), NULL);
        functionLayout->addWidget(this->ManualFunctionWidget);
        break;
      case cmbProfileFunction::WEDGE:
        WedgeFunctionWidget =
          new qtCMBProfileWedgeFunctionWidget(NULL, dynamic_cast<cmbProfileWedgeFunction*>(fun));
        functionLayout->addWidget(this->WedgeFunctionWidget);
        break;
    }
  }
}

void pqCMBModifierArcManager::functionTypeChanged(int type)
{
  if(selectedFunction == NULL) return;
  if(type == selectedFunction->getType()) return;
  cmbProfileFunction* fun = NULL;
  std::string name = selectedFunction->getName();
  switch( type )
  {
    case cmbProfileFunction::MANUAL:
      fun = new cmbManualProfileFunction();
      break;
    case cmbProfileFunction::WEDGE:
      fun = new cmbProfileWedgeFunction();
      break;
  }
  this->CurrentModifierArc->setFunction(name, fun);
  int	index = this->UI->FunctionName->currentIndex();
  if(index != -1)
  {
    QVariant vdata;
    vdata.setValue(static_cast<void*>(fun));
    this->UI->FunctionName->setItemData(index, vdata); //todo
  }

  selectFunction(fun);
}

void pqCMBModifierArcManager::doneModifyingArc()
{
  if(this->CurrentModifierArc != NULL)
    {
    foreach(QString filename, ServerProxies.keys())
      {
      foreach(int pieceIdx, ServerProxies[filename].keys())
        {
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
        this->CurrentModifierArc->updateArc(source);
        }
      }
    }

  this->enableSelection();
  this->unselectAllRows();
  emit this->modifyingArcDone();
  selectLine(-1);
}

void pqCMBModifierArcManager::addApplyControl()
{
  QMap< QString, QMap<int, bool> > newVis;
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      newVis[filename][pieceIdx] = true;
      }
    }
  ArcLinesApply.resize(this->ArcLines.size(), newVis);
}

void pqCMBModifierArcManager::sendOrder()
{
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      std::vector<int> order = this->getCurrentOrder(filename, pieceIdx);
      QList< QVariant > v;
      v << static_cast<int>(order.size());
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ResizeOrder"), v);
      source->UpdateVTKObjects();
      v.clear();
      v << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ResizeOrder"), v);
      source->UpdateVTKObjects();
      for (unsigned int i = 0; i < order.size(); ++i)
        {
        v.clear();
        v << static_cast<int>(i) << order[i];
        pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetOrderValue"), v);
        source->UpdateVTKObjects();
        }
      v.clear();
      v << -1 << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetOrderValue"), v);
      source->UpdateVTKObjects();
      source->UpdatePipeline();
      source->UpdatePropertyInformation();
      }
    }
  emit requestRender();
}

void pqCMBModifierArcManager::setUpTable()
{
  QTableWidget* tmp = this->UI->DatasetTable;
  tmp->blockSignals(true);
  tmp->clearContents();
  tmp->setRowCount(0);
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  tmp->setSelectionMode(QAbstractItemView::NoSelection);
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      tmp->insertRow(tmp->rowCount());
      int row =  tmp->rowCount()-1;
      ServerRows[filename][pieceIdx] = row;
      QTableWidgetItem* objItem = new QTableWidgetItem();
      QVariant vdata;
      objItem->setData(Qt::UserRole, vdata);

      tmp->setItem(row, 0, objItem);
      objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
      objItem->setCheckState(Qt::Checked);

      objItem = new QTableWidgetItem(filename);
      tmp->setItem(row, 1, objItem);
      objItem->setFlags(commFlags);

      objItem = new QTableWidgetItem( QString::number(pieceIdx) );
      tmp->setItem(row, 2, objItem);
      objItem->setFlags(commFlags);
      }
    }
  tmp->resizeColumnsToContents();
  tmp->blockSignals(false);
}

void pqCMBModifierArcManager::setUpPointsTable()
{
  pqCMBModifierArc::pointFunctionWrapper * wrapper = NULL;
  QList<QTableWidgetItem *>selected =	this->UI->points->selectedItems();
  if( !selected.empty() )
  {
    QVariant qv = selected[0]->data(Qt::UserRole);
    wrapper = static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
  }
  QTableWidget* tmp = this->UI->points;
  tmp->setSelectionMode(QAbstractItemView::SingleSelection);
  tmp->setSelectionBehavior(QAbstractItemView::SelectRows);
  tmp->blockSignals(true);

  tmp->clearContents();
  tmp->setRowCount(0);
  if(CurrentModifierArc == NULL)
  {
    tmp->blockSignals(false);
    return;
  }
  tmp->setSelectionMode(QAbstractItemView::SingleSelection);

  pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
  vtkPVArcInfo* ai = arc->getArcInfo();
  std::vector<pqCMBModifierArc::pointFunctionWrapper const*> points;
  CurrentModifierArc->getPointFunctions(points);
  for(size_t i = 0; i < points.size(); ++i)
  {
    this->addItemToTable(points[i], wrapper == points[i]);
  }
  tmp->resizeColumnsToContents();
  tmp->blockSignals(false);
}

void pqCMBModifierArcManager::setDatasetTable(int inId)
{
  if(inId < 0 || static_cast<size_t>(inId) >= ArcLines.size()) return;
  this->UI->DatasetTable->blockSignals(true);
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      int row = ServerRows[filename][pieceIdx];
      bool visOnDs = ArcLinesApply[inId][filename][pieceIdx];
      this->UI->DatasetTable->item( row, 0 )->setCheckState( visOnDs ? Qt::Checked : Qt::Unchecked);
      }
    }
  this->UI->DatasetTable->blockSignals(false);
}

void pqCMBModifierArcManager::disableAbsolute()
{
  for(int i = 0; i < this->TableWidget->rowCount(); ++i)
  {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    int id = tmp->text().toInt();
    pqCMBModifierArc * ma = ArcLines[id];
    if(ma != NULL)
    {
      //TODO
      //ma->setRelative(true);
      this->setRow(i,ma);
    }
  }
}

void pqCMBModifierArcManager::enableAbsolute()
{
  //TODO
}

void pqCMBModifierArcManager::onSaveProfile()
{
  if(this->CurrentModifierArc != NULL)
  {
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                                                    "",
                                                    tr("Function Profile (*.fpr)"));
    if(fileName.isEmpty())
    {
      return;
    }
    std::ofstream out(fileName.toStdString().c_str());
    this->CurrentModifierArc->writeFunction(out);
  }
}

void pqCMBModifierArcManager::onLoadProfile()
{
  if(this->CurrentModifierArc != NULL)
  {
    QStringList fileNames =
      QFileDialog::getOpenFileNames(NULL, "Open File...", "", "Function Profile (*.fpr)");
    if(fileNames.count()==0)
    {
      return;
    }
    std::string fname = fileNames[0].toStdString();
    std::ifstream in(fname.c_str());
    this->CurrentModifierArc->readFunction(in);
  }
}

void pqCMBModifierArcManager::onSaveArc()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                                                  "",
                                                  tr("Function Profile (*.mar)"));
  if(fileName.isEmpty())
  {
    return;
  }

  QList<pqOutputPort*> ContourInputs;
  std::ofstream out(fileName.toStdString().c_str());
  out << 1 << "\n";
  out << ArcLines.size() << "\n";

  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    ArcLines[i]->write(out);
    pqOutputPort *port = ArcLines[i]->GetCmbArc()->getSource()->getOutputPort(0);
    ContourInputs.push_back( port );
  }
  {
    QFileInfo fi(fileName);
    std::string fname = QFileInfo(fi.dir(), fi.baseName()+".vtp").absoluteFilePath().toStdString();
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = ContourInputs;

    //now that we have collected all the contour info, write out the file
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    pqPipelineSource* writer = builder->createFilter("writers",
                                                     "SceneGenV2ContourWriter",
                                                     namedInputs, core->getActiveServer());

    vtkSMSourceProxy *proxy = vtkSMSourceProxy::SafeDownCast(writer->getProxy());

    pqSMAdaptor::setElementProperty( proxy->GetProperty("FileName"),
                                     fname.c_str() );
    proxy->UpdateProperty("FileName");
    proxy->UpdatePipeline();

    //now that the file has been written, delete the writer
    builder->destroy(writer);
  }
}

void pqCMBModifierArcManager::onLoadArc()
{
  QStringList fileNames =
  QFileDialog::getOpenFileNames(NULL, "Open File...", "", "Function Profile (*.mar)");
  if(fileNames.count()==0)
  {
    return;
  }

  std::string fname = fileNames[0].toStdString();
  unsigned int start = ArcLines.size();
  int rc = this->TableWidget->rowCount();
  {
    QFileInfo fi(fileNames[0]);
    pqPipelineSource *reader = NULL;
    QStringList files;
    files << QFileInfo(fi.dir(), fi.baseName()+".vtp").absoluteFilePath();

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    builder->blockSignals(true);

    reader = builder->createReader("sources", "XMLPolyDataReader", files,
                                   core->getActiveServer());
    builder->blockSignals(false);

    if (reader)
    {
      pqPipelineSource* extractContour = builder->createFilter("filters",
                                                               "CmbExtractContours", reader);
      vtkSMSourceProxy *proxy = vtkSMSourceProxy::SafeDownCast( extractContour->getProxy() );
      proxy->UpdatePipeline();
      proxy->UpdatePropertyInformation();
      int max =pqSMAdaptor::getElementProperty(proxy->GetProperty("NumberOfContoursInfo")).toInt();
      pqCMBModifierArc * dig = new pqCMBModifierArc(proxy);
      addNewArc(dig);
      for ( int i = 1; i < max; ++i)
      {
        pqSMAdaptor::setElementProperty(proxy->GetProperty("ContourIndex"),i);
        proxy->UpdateProperty("ContourIndex");
        proxy->UpdatePipeline();
        pqCMBModifierArc * dig = new pqCMBModifierArc(proxy);
        QObject::connect( dig, SIGNAL(requestRender()),
                          this, SIGNAL(requestRender()) );
        addNewArc(dig);
      }
    }
  }
  std::ifstream in(fname.c_str());
  int version;
  in >> version;
  unsigned int num;
  in >> num;
  for(unsigned int i = 0; i < num; ++i)
  {
    unsigned int at = start + i;
    ArcLines[at]->read(in);
    this->setRow( rc+i, ArcLines[at] );
    ArcLines[at]->switchToNotEditable();
  }
  accepted();
  onClearSelection();
  this->CurrentModifierArc = NULL;
  this->check_save();
}

void pqCMBModifierArcManager::check_save()
{
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    if(ArcLines[i] != NULL)
    {
      this->UI->SaveFunctions->setEnabled(true);
      return;
    }
  }
  this->UI->SaveFunctions->setEnabled(false);
}


void pqCMBModifierArcManager::nameChanged(QString n)
{
  if(selectedFunction == NULL || CurrentModifierArc == NULL) return;
  if(n.isEmpty()) return;

  if(CurrentModifierArc->updateLabel(n.toStdString(), selectedFunction))
  {
    int	index = this->UI->FunctionName->currentIndex();
    if(index != -1)
    {
      this->UI->FunctionName->setItemText(index, n);
    }
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::white);
    palette.setColor(QPalette::Text,Qt::black);
    this->UI->FunctionName->setPalette(palette);
  }
  else
  {
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::red);
    palette.setColor(QPalette::Text,Qt::black);
    this->UI->FunctionName->setPalette(palette);
  }
}

void pqCMBModifierArcManager::setToDefault()
{
  //TODO redo this
  if(selectedFunction && CurrentModifierArc)
  {
    CurrentModifierArc->setStartFun(selectedFunction->getName());
  }
}

void pqCMBModifierArcManager::newFunction()
{
  if(CurrentModifierArc)
  {
    cmbProfileFunction * nfun = CurrentModifierArc->createFunction();
    int	count = this->UI->FunctionName->count();
    this->AddFunction(nfun);
    this->UI->FunctionName->setCurrentIndex(count);
  }
}

void pqCMBModifierArcManager::deleteFunction()
{
  if(selectedFunction && CurrentModifierArc)
  {
    if(CurrentModifierArc->deleteFunction(selectedFunction->getName()))
    {
      int	index = this->UI->FunctionName->currentIndex();
      if(index != -1)
      {
        this->UI->FunctionName->removeItem(index);
        if(index != 0)
        {
          index--;
        }
        this->UI->FunctionName->setCurrentIndex(index);
      }
      onFunctionSelectionChange();
    }
  }
  this->UI->DeleteFunction->setEnabled(this->UI->FunctionName->count() > 1);
}

void pqCMBModifierArcManager::cloneFunction()
{
  if(selectedFunction && CurrentModifierArc)
  {
    int	count = this->UI->FunctionName->count();
    this->AddFunction(CurrentModifierArc->cloneFunction(selectedFunction->getName()));
    this->UI->FunctionName->setCurrentIndex(count);
  }
}

void pqCMBModifierArcManager::functionModeChanged(int m)
{
  if(CurrentModifierArc == NULL) return;
  pqCMBModifierArc::FunctionMode mode = static_cast<pqCMBModifierArc::FunctionMode>(m);
  CurrentModifierArc->setFunctionMode(mode);
  this->UI->EndPointDisplayed->hide();
  this->UI->pointsFrame->hide();
  switch(CurrentModifierArc->getFunctionMode())
  {
    case pqCMBModifierArc::PointAssignment:
      this->UI->pointsFrame->show();

      break;
    case pqCMBModifierArc::Single:
      this->pointDisplayed(0);
      //this->ArcWidgetManager->highlightPoint(-1);
      break;
    case pqCMBModifierArc::EndPoints:
      this->pointDisplayed(this->UI->EndPointDisplayed->currentIndex());
      this->UI->EndPointDisplayed->show();

      break;
  }
}

void pqCMBModifierArcManager::pointDisplayed(int index)
{
  QString name;
  if(CurrentModifierArc == NULL) return;
  switch(CurrentModifierArc->getFunctionMode())
  {
    case pqCMBModifierArc::EndPoints:
      if(index == 1)
      {
        name = CurrentModifierArc->getEndFun()->getName().c_str();
        break;
      }
    case pqCMBModifierArc::Single:
      //this->ArcWidgetManager->highlightPoint(0);
      name = CurrentModifierArc->getStartFun()->getName().c_str();
      break;
    case pqCMBModifierArc::PointAssignment:
      break;
  }
  this->UI->FunctionName->setCurrentIndex(this->UI->FunctionName->findText(name));
}

void pqCMBModifierArcManager::addPoint(vtkIdType id)
{
  this->addPointMode = false;
  std::vector<cmbProfileFunction*> funs;
  CurrentModifierArc->getFunctions(funs);
  if(!CurrentModifierArc->pointHasFunction(id))
  {
    this->addItemToTable(CurrentModifierArc->addFunctionAtPoint(id, funs[0]), true);
  }
}

void pqCMBModifierArcManager::addPoint()
{
  ArcWidgetManager->startSelectPoint();
  this->addPointMode = true;
}

void pqCMBModifierArcManager::deletePoint()
{
  if(this->addPointMode)
  {
    ArcWidgetManager->cancelSelectPoint();
    this->addPointMode = false;
    return;
  }

  QList<QTableWidgetItem *>selected =	this->UI->points->selectedItems();
  if( !selected.empty() )
  {
    QVariant qv = selected[0]->data(Qt::UserRole);
    pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
    CurrentModifierArc->removeFunctionAtPoint(wrapper->getPointId());
    this->UI->points->removeRow(selected[0]->row());

  }
}

void pqCMBModifierArcManager::addItemToTable(pqCMBModifierArc::pointFunctionWrapper const* mp,
                                             bool select)
{
  if(mp == NULL) return;
  QTableWidget* tmp = this->UI->points;

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  double pt[3];
  vtkPVArcInfo* ai =  CurrentModifierArc->GetCmbArc()->getArcInfo();
  ai->GetPointLocation(mp->getPointIndex(), pt);

  int row = tmp->rowCount();
  if(tmp->rowCount() != 0)
  {
    //If there are enough points, binary would be significantly faster
    for(row = 0; row < tmp->rowCount(); ++row)
    {
      QTableWidgetItem * item = tmp->item(row, 0);
      QVariant qv = item->data(Qt::UserRole);
      pqCMBModifierArc::pointFunctionWrapper * wrapper =
                      static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
      if(mp->getPointIndex() < wrapper->getPointIndex()) break;
    }
  }

  tmp->insertRow(row);

  QTableWidgetItem * qtwi = new QTableWidgetItem(QString::number(mp->getPointId()));
  QVariant vdata;
  vdata.setValue(static_cast<void*>(const_cast<pqCMBModifierArc::pointFunctionWrapper *>(mp)));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 1, qtwi);
  qtwi = new QTableWidgetItem(QString::number(pt[0]));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 2, qtwi);
  qtwi = new QTableWidgetItem(QString::number(pt[1]));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 3, qtwi);
  qtwi = new QTableWidgetItem(QString(mp->getName().c_str()));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 4, qtwi);
  qtwi = new QTableWidgetItem(QString::number(mp->getPointIndex()));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 0, qtwi);
  if(select)
  {
    tmp->selectRow(row);
  }
  if (tmp->rowCount() == 1)
  {
    tmp->resizeColumnsToContents();
  }
}
