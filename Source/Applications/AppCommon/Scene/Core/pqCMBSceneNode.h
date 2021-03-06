//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneNode - represents a node in a Scene Tree.
// .SECTION Description
// .SECTION Caveats

#ifndef __SceneNode_h
#define __SceneNode_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "vtkBoundingBox.h"
#include <QColor>
#include <string>
#include <vector>

class pqCMBSceneTree;
class pqCMBSceneObjectBase;
class QTreeWidgetItem;
class digLine;

class CMBAPPCOMMON_EXPORT pqCMBSceneNode
{
  friend class pqCMBSceneTree;
  friend class digLine;
  friend class pqCMBSceneNodeCreationEvent;
  friend class pqCMBSceneNodeDeletionEvent;
  friend class cmbSceneNodeReplaceEvent;

public:
  const char* getName() const { return this->Name.c_str(); }
  pqCMBSceneObjectBase* getDataObject() const { return this->Object; }
  pqCMBSceneTree* getTree() const { return this->Tree; }
  const std::vector<pqCMBSceneNode*>& getChildren() const { return this->Children; }
  QTreeWidgetItem* getWidget() const { return this->Widget; }
  QTreeWidgetItem* getInfoWidget() const { return this->InfoWidget; }

  void removeChild(pqCMBSceneNode* child, bool deleteNode = false);
  void setVisibility(bool mode);
  void toggleVisibility();
  bool isVisible() const { return this->Visible; }

  void setIsLocked(bool mode);
  void toggleLocked();
  bool isLocked() const { return this->IsLocked; }

  bool isTypeNode() const { return this->Object == NULL; }
  bool isRootNode() const { return this->Parent == NULL; }
  bool isArcNode();
  bool isLineNode();
  bool isSelected() const { return this->Selected; }
  bool isAncestorWidgetSelected() const;
  void select();
  void unselect();
  bool hasExplicitColor() const { return this->ExplicitColor; }
  void setExplicitColor(double color[4]);
  void setExplicitColor(QColor& color);
  void unsetExplicitColor();
  void collapseAllDataInfo();
  void recomputeInfo(QTreeWidgetItem*);
  void getColor(double color[4]) const
  {
    color[0] = this->NodeColor[0];
    color[1] = this->NodeColor[1];
    color[2] = this->NodeColor[2];
    color[3] = this->NodeColor[3];
  }
  pqCMBSceneNode* getParent() const { return this->Parent; }
  void setMeshOutputIndex(int meshIndex);
  int getMeshOutputIndex() const { return this->MeshOutputIndex; }
  void zoomOnData();

  bool getBounds(vtkBoundingBox*) const;
  bool isMarkedForDeletion() const { return this->MarkedForDeletion; }
  static pqCMBSceneNode* getNodeFromWidget(QTreeWidgetItem* widget);
  static int getNameColumn() { return 0; }
  static int getVisibilityColumn() { return 1; }
  static int getLockColumn() { return 2; }
protected:
  pqCMBSceneNode(const char* name, pqCMBSceneTree* tree);
  pqCMBSceneNode(const char* name, pqCMBSceneNode* parent, pqCMBSceneObjectBase* obj = NULL);
  ~pqCMBSceneNode();
  void addChild(pqCMBSceneNode* child);
  void init();
  void setParent(pqCMBSceneNode* parent);
  void changeName(const char* newName);
  void setSelected(bool mode);
  void setParentVisibilityOn();
  void unlockParent();
  void pushColor();
  void renameInfoNodes();
  void populateInfoNodes();
  void setMarkedForDeletion();
  void unsetMarkedForDeletion();
  void replaceObject(pqCMBSceneObjectBase* object);
  std::string Name;
  pqCMBSceneObjectBase* Object;
  std::vector<pqCMBSceneNode*> Children;
  QTreeWidgetItem* Widget;
  QTreeWidgetItem* InfoWidget;
  pqCMBSceneTree* Tree;
  pqCMBSceneNode* Parent;
  bool Visible;
  bool IsLocked;
  bool Selected;
  bool BeingDeleted;
  bool MarkedForDeletion;
  bool ExplicitColor;
  double NodeColor[4];
  int MeshOutputIndex;
};

#endif /* SceneNode_h */
