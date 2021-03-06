//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __qtCMBArcEditWidget_h
#define __qtCMBArcEditWidget_h

#include "cmbSystemConfig.h"
#include "vtkCommand.h"
#include "vtkProp.h"
#include "vtkPropPicker.h"
#include "vtkType.h"
#include <QAction>        //needed for ArcPointPicker
#include <QStringBuilder> //needed for more efficient string concatenating
#include <QWidget>

class pqOutputPort;
class pqRenderView;
class pqRenderViewSelectionReaction;
class pqCMBArc;
class qtArcWidget;
class qtCMBArcWidgetManager;
class qtCMBArcEditWidget;

namespace Ui
{
class qtCMBArcEditWidget;

struct PickInfo
{
  bool IsValid;
  double pointLocation[3];
  pqOutputPort* port;
  vtkIdType PointId;

  PickInfo()
    : IsValid(false)
    , PointId(-1)
  {
  }

  QString text() const
  {
    return QString("Id: ") % QString::number(PointId) % QString("; Position: ") %
      QString::number(pointLocation[0]) % ", " % QString::number(pointLocation[1]) % ", " %
      QString::number(pointLocation[2]);
  }
};

class ArcPointPicker : public QAction
{
  Q_OBJECT

public:
  ArcPointPicker(QObject* parent);
  ~ArcPointPicker() override;
signals:
  //called by the selector when a valid selection is finished.
  void pickFinished();
  //emitted to allow selection to happen
  void triggered(bool);

public slots:
  void doPick(pqRenderView* view, pqCMBArc* arc, PickInfo& info);

protected slots:
  //saves the information returned from the selection.
  void selectedInfo(pqOutputPort* port);
  // picking arc end point finished
  void onPickingFinished();

private:
  PickInfo* Info;
  pqCMBArc* Arc;
  pqRenderView* View;
  pqRenderViewSelectionReaction* Selecter;
};
}

class vtkPointSelectedCallback;
class pqCmbModifierArcManager;

class qtCMBArcEditWidget : public QWidget
{
  Q_OBJECT

public:
  friend class vtkPointSelectedCallback;
  friend class qtCMBArcWidgetManager;
  friend class pqCmbModifierArcManager;

  explicit qtCMBArcEditWidget(QWidget* parent = 0);
  ~qtCMBArcEditWidget() override;

  virtual void setView(pqRenderView* view) { this->View = view; }
  virtual void setArc(pqCMBArc* arc);
  virtual void setArcManager(qtCMBArcWidgetManager* arcManager) { this->ArcManager = arcManager; }
  // is the sub-arc valid
  bool isSubArcValid();
  // if the whole arc is selected, the original contour widget panel
  // will be shown. Otherwise the new sub-arc editing panel will be shown
  bool isWholeArcSelected();

  void selectPointMode();

signals:
  void arcModified(qtArcWidget*, vtkIdType, vtkIdType);
  void arcModificationfinished();
  void startArcEditing();
  void selectedPointOnLine(vtkIdType);

protected slots:
  //shows the edit widget and hides the pick widget
  void showEditWidget();

  //shows the pick widget and hides the edit widget
  void showPickWidget();
  //called when arc editing is done
  void arcEditingFinished();
  //marks that we are finished editing this arc
  void finishedArcModification();

  //marks that that we don't want to save the modifications
  //to the arc
  void cancelEdit();

  // save the modified sub-arc to original arc. This will
  // replace all the points in the arc (StartPoint-to-EndPoint)
  // with the points on the arc widget.
  void saveEdit();

  //allows the user to select the start position of the arc.
  //will record the position that is selected
  void PickStartPoint();

  //allows the user to select the end position of the arc.
  //will record the position that is selected
  void PickEndPoint();

  // start modifying the selected sub-arc.
  void modifySubArc();

  // remove all the nodes of the selected sub-arc
  void onCollapseSubArc();

  // clear all the internal nodes of the selected the sub-arc.
  void onStraightenArc();

  // make arcs out of the sub-arc.
  void onMakeArc();

  // hide the arc editing widget
  void hideArcWidget();

  // pick the whole arc for operations
  void pickWholeArc();

  void selectedPoint(int);

private:
  //resets the widget to what it would be like if it was just created
  void resetWidget();
  // this will create a representation for the specified sub arc
  void updateSubArcRepresentation(bool visible);
  // update the existing arc representation.
  void updateWholeArcRepresentation(bool visible);
  // set visibility of the sub arc representation
  void setSubArcVisible(int visible);

  class pqInternals;
  pqInternals* Internals;

  Ui::ArcPointPicker Picker;
  pqRenderView* View;
  pqCMBArc* Arc;
  qtArcWidget* SubWidget;
  qtCMBArcWidgetManager* ArcManager;

  Ui::PickInfo StartPoint;
  Ui::PickInfo EndPoint;
};

#endif // __qtCMBArcEditWidget_h
