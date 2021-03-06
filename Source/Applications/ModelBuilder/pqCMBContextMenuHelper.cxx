//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBContextMenuHelper.h"
#include "SimBuilder/pqSMTKUIHelper.h"
#include "pqCMBModelManager.h"
#include "pqDataRepresentation.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqSMAdaptor.h"
#include "pqSMTKMeshInfo.h"
#include "pqSMTKModelInfo.h"

#include "vtkDataObject.h"
#include "vtkPVSMTKMeshInformation.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkPVSelectionInformation.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkSMViewProxy.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Volume.h"

#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

const std::string pqCMBContextMenuHelper::s_internal_groupOpName = "entity group";

bool pqCMBContextMenuHelper::getBlockIndex(const smtk::model::EntityRef& eref, vtkIdType& flatIndex)
{
  const smtk::model::IntegerList& prop(eref.integerProperty("block_index"));
  if (!prop.empty() && prop[0] >= 0)
  {
    flatIndex = prop[0];
    return true;
  }
  return false;
}

/// Fetch children for volum and group entities.
void pqCMBContextMenuHelper::accumulateChildGeometricEntities(
  QSet<vtkIdType>& blockIds, const smtk::model::EntityRef& toplevel)
{
  vtkIdType bidx = -1;
  if (toplevel.isVolume() && !toplevel.hasIntegerProperty("block_index"))
  { // Add free cells
    smtk::model::Faces faces = toplevel.as<smtk::model::Volume>().faces();
    // Find all boundaries of all free cells
    smtk::model::Faces::iterator fcit;
    for (fcit = faces.begin(); fcit != faces.end(); ++fcit)
    {
      if (fcit->hasIntegerProperty("block_index") && getBlockIndex(*fcit, bidx))
        blockIds.insert(bidx);

      // for its edges and vertices
      smtk::model::EntityRefs bdys = fcit->lowerDimensionalBoundaries(-1); // Get *all* boundaries.
      smtk::model::EntityRefs::const_iterator evit;
      for (evit = bdys.begin(); evit != bdys.end(); ++evit)
        if (evit->hasIntegerProperty("block_index") && getBlockIndex(*evit, bidx))
          blockIds.insert(bidx);
    }
  }
  else if (toplevel.isGroup())
  { // Add group members, but not their boundaries
    smtk::model::EntityRefs members =
      toplevel.as<smtk::model::Group>().members<smtk::model::EntityRefs>();
    for (smtk::model::EntityRefs::const_iterator it = members.begin(); it != members.end(); ++it)
      // Do this recursively since a group may contain other groups
      accumulateChildGeometricEntities(blockIds, *it);
  }

  // make sure the volume or group is checked if they have geometry themselves
  if (toplevel.hasIntegerProperty("block_index") && getBlockIndex(toplevel, bidx))
    blockIds.insert(bidx);
}

// only use valid color, the rest will be colored by default color
bool pqCMBContextMenuHelper::getValidEntityColor(
  QColor& color, const smtk::model::EntityRef& entref)
{
  smtk::model::FloatList rgba = entref.color();
  if (rgba.size() == 4 && (rgba[0] >= 0. && rgba[0] <= 1.0 && rgba[1] >= 0. && rgba[1] <= 1.0 &&
                            rgba[2] >= 0. && rgba[2] <= 1.0 && (rgba[3] >= 0. && rgba[3] <= 1.0)))
  {
    color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
    return true;
  }
  // If the color is invalid, it might got assigned by ENTITY_LIST. Look up the
  // default color from the entity's model properties.
  if (entref.owningModel().isValid())
  {
    smtk::model::Model model = entref.owningModel();
    std::string colorName = smtk::model::Entity::flagSummary(entref.entityFlags());
    colorName += " color";
    if (model.hasFloatProperty(colorName))
    {
      smtk::model::FloatList rgbaModel = model.floatProperty(colorName);
      // Check if user has invalidate the entity list color.
      // Refer to qtModelView::changeEntityColor for detail
      if (rgbaModel.size() == 4 && rgbaModel[3] != -2.0)
      {
        float alpha = rgbaModel.size() == 4 ? std::max(0., std::min(rgbaModel[3], 1.0)) : 1.;
        // alpha can't be zero
        alpha = alpha == 0. ? 1.0 : alpha;
        color.setRgbF(rgbaModel[0], rgbaModel[1], rgbaModel[2], alpha);
        return true;
      }
    }
  }
  // Use default color specified by user in the settings when entref does not
  // have a color or user resets it to default color
  // FIXME: For now there is no way to remove string property on the client side,
  if (!entref.hasColor() || (rgba.size() == 4 && rgba[3] == -1))
  {
    if (entref.isFace())
    {
      color = smtk::extension::QEntityItemModel::defaultEntityColor("Face");
      return color.isValid() ? true : false;
    }
    else if (entref.isEdge())
    {
      color = smtk::extension::QEntityItemModel::defaultEntityColor("Edge");
      return color.isValid() ? true : false;
    }
    else if (entref.isVertex())
    {
      color = smtk::extension::QEntityItemModel::defaultEntityColor("Vertex");
      return color.isValid() ? true : false;
    }
  }
  return false;
}

// only use valid color, the rest will be colored
// randomly with CTF
bool pqCMBContextMenuHelper::getValidMeshColor(QColor& color, const smtk::mesh::MeshSet& mesh)
{
  smtk::mesh::CollectionPtr c = mesh.collection();
  if (c->isValid() && c->hasFloatProperty(mesh, "color"))
  {
    smtk::model::FloatList rgba = c->floatProperty(mesh, "color");
    if ((rgba.size() == 3 || rgba.size() == 4) &&
      (rgba[0] >= 0. && rgba[0] <= 1.0 && rgba[1] >= 0. && rgba[1] <= 1.0 && rgba[2] >= 0. &&
          rgba[2] <= 1.0 && rgba[0] + rgba[1] + rgba[2] != 0))
    {
      float alpha = rgba.size() == 4 ? std::max(0., std::min(rgba[3], 1.0)) : 1.;
      color.setRgbF(rgba[0], rgba[1], rgba[2], alpha);
      return true;
    }
  }
  return false;
}

// only use valid color, the rest will be colored
// randomly with CTF
bool pqCMBContextMenuHelper::validMeshColorMode(
  const QString& colorMode, const smtk::mesh::MeshSet& mesh)
{
  smtk::model::EntityRefArray meshEntRefs;
  mesh.modelEntities(meshEntRefs);
  if (meshEntRefs.size() == 0)
    return false;

  smtk::model::EntityRef entref = meshEntRefs[0];
  return (colorMode == vtkModelMultiBlockSource::GetVolumeTagName() && entref.isVolume()) ||
    (colorMode == vtkModelMultiBlockSource::GetGroupTagName() &&
           entref.isGroup()) || // should we check memebers
    (colorMode == vtkModelMultiBlockSource::GetEntityTagName() && !entref.isVolume() &&
           !entref.isGroup());
}

// return total number of blocks selected

int pqCMBContextMenuHelper::getSelectedRepBlocks(const QList<pqSMTKModelInfo*>& selModels,
  const QList<pqSMTKMeshInfo*>& selMeshes, QMap<pqSMTKModelInfo*, QList<vtkIdType> >& modelresult,
  QMap<pqSMTKMeshInfo*, QList<vtkIdType> >& meshresult)
{
  int totalBlocks = 0;
  foreach (pqSMTKModelInfo* modinfo, selModels)
  {
    pqPipelineSource* source = modinfo->RepSource;
    vtkNew<vtkPVSelectionInformation> selInfo;
    if (vtkSelectionNode* selnode =
          pqCMBSelectionHelperUtil::gatherSelectionNode(source, selInfo.GetPointer()))
    {
      vtkUnsignedIntArray* blockIds = selnode->GetSelectionList()
        ? vtkUnsignedIntArray::SafeDownCast(selnode->GetSelectionList())
        : NULL;
      if (blockIds)
      {
        totalBlocks +=
          pqCMBSelectionHelperUtil::fillSelectionIdList(modelresult[modinfo], blockIds, NULL);
      }
      // this may be the a ID selection
      else
      {
        vtkSMSourceProxy* selSource =
          vtkSMSourceProxy::SafeDownCast(source->getProxy())->GetSelectionInput(0);
        vtkSMPropertyHelper selIDs(selSource, "IDs");
        totalBlocks +=
          pqCMBSelectionHelperUtil::fillSelectionIdList(modelresult[modinfo], NULL, &selIDs);
      }
    }
  }

  foreach (pqSMTKMeshInfo* meshinfo, selMeshes)
  {
    pqPipelineSource* source = meshinfo->RepSource;
    vtkNew<vtkPVSelectionInformation> selInfo;
    if (vtkSelectionNode* selnode =
          pqCMBSelectionHelperUtil::gatherSelectionNode(source, selInfo.GetPointer()))
    {
      vtkUnsignedIntArray* blockIds = selnode->GetSelectionList()
        ? vtkUnsignedIntArray::SafeDownCast(selnode->GetSelectionList())
        : NULL;
      if (blockIds)
      {
        totalBlocks +=
          pqCMBSelectionHelperUtil::fillSelectionIdList(meshresult[meshinfo], blockIds, NULL);
      }
      // this may be the a ID selection
      else
      {
        vtkSMSourceProxy* selSource =
          vtkSMSourceProxy::SafeDownCast(source->getProxy())->GetSelectionInput(0);
        vtkSMPropertyHelper selIDs(selSource, "IDs");
        totalBlocks +=
          pqCMBSelectionHelperUtil::fillSelectionIdList(meshresult[meshinfo], NULL, &selIDs);
      }
    }
  }

  return totalBlocks;
}

bool pqCMBContextMenuHelper::hasSessionOp(
  const smtk::model::SessionPtr& brSession, const std::string& opname)
{
  if (brSession)
  {
    smtk::model::StringList opNames = brSession->operatorNames();
    return std::find(opNames.begin(), opNames.end(), opname) != opNames.end();
  }
  return false;
}

bool pqCMBContextMenuHelper::startGroupOp(pqCMBModelManager* modMgr, pqSMTKModelInfo* minfo,
  const std::string& optype, const QList<vtkIdType>& addblocks,
  const smtk::model::Group& modifyGroup)
{
  smtk::model::ManagerPtr mgr = modMgr->managerProxy()->modelManager();
  smtk::model::SessionRef sessionRef(mgr, minfo->SessionId);
  smtk::model::SessionPtr sessPtr = sessionRef.session();
  if (sessPtr && hasSessionOp(sessPtr, s_internal_groupOpName))
  {
    // create the group operator, if the session has one
    smtk::model::OperatorPtr grpOp = sessPtr->op(s_internal_groupOpName);
    if (!grpOp)
    {
      std::cout << "Could not create operator: \"" << s_internal_groupOpName << "\" for session"
                << " \"" << sessPtr->name() << "\""
                << " (" << sessPtr->sessionId() << ")\n";
      return false;
    }
    // set up group operator with selected entities. Currently, only
    // the discrete session has the "entity group" operator
    smtk::attribute::AttributePtr attrib = grpOp->specification();
    smtk::attribute::ModelEntityItemPtr modelItem = attrib->findModelEntity("model");
    smtk::attribute::StringItem::Ptr optypeItem = attrib->findString("Operation");
    smtk::attribute::ModelEntityItemPtr grpItem = attrib->findAs<smtk::attribute::ModelEntityItem>(
      "modify cell group", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::ModelEntityItemPtr addItem = attrib->findAs<smtk::attribute::ModelEntityItem>(
      "cell to add", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::IntItem::Ptr grptypeItem = attrib->findInt("group type");
    smtk::attribute::StringItem::Ptr grpnameItem = attrib->findString("group name");
    if (!modelItem || !optypeItem || !grpItem || !addItem || !grptypeItem || !grpnameItem)
    {
      std::cout
        << "The entity group operator's specification is missing items!\n"
        << "For reference, checkout smtk/bridge/discrete/operators/EntityGroupOperator.sbt.\n";
      return false;
    }

    smtk::common::UUID modelId = minfo->Info->GetModelUUID();
    smtk::model::Model activeModel(mgr, modelId);
    if (!activeModel.isValid())
    {
      std::cout << "Could not find model with UUID: " << modelId << "\n";
      return false;
    }
    modelItem->setValue(activeModel);
    optypeItem->setValue(optype.c_str());

    // Due to limitations in the underlying group operation in "discrete" modeling kernel:
    // For 2D model boundary group, only edges are allowed; 2D model faces are only allowed for domain group.
    // For 3D model boundary group, only faces are allowed; domain group can only contained volumes (regions).
    // Therefore, based on the highest dimension selected, we will have above contraints to set up the group op.
    const smtk::attribute::ModelEntityItemDefinition* addItemDef =
      dynamic_cast<const smtk::attribute::ModelEntityItemDefinition*>(addItem->definition().get());

    int dim = activeModel.dimension();
    bool hasFace = false, hasVol = false;
    smtk::model::EntityRefArray selEntRefs;
    smtk::common::UUID uid;
    foreach (vtkIdType bid, addblocks)
    {
      uid = minfo->Info->GetModelEntityId(bid);
      smtk::model::EntityRef entref(mgr, uid);
      if (addItemDef->isValueValid(entref))
      {
        selEntRefs.push_back(entref);
        if (!hasFace)
        {
          hasFace = entref.isFace();
        }
        if (!hasVol)
        {
          hasVol = entref.isVolume();
        }
      }
    }

    if (selEntRefs.size() > 0 && !addItem->setValues(selEntRefs.begin(), selEntRefs.end()))
    {
      std::cout << "setNumberOfValues failed for \"cell to add\" item!\n";
      return false;
    }

    if (optype == "Create")
    {
      grpnameItem->setValue("boundary group");
      // by default, group type is set to 0 (boundary) for new group
      if ((dim == 2 && hasFace) || (dim == 3 && hasVol))
      {
        grptypeItem->setValue(1); // domain
        grpnameItem->setValue("domain group");
      }
    }
    else if (optype == "Modify")
    {
      if (!modifyGroup.isValid() || addItem->numberOfValues() == 0)
      {
        std::cout
          << "Either the input group is not valid, or no entities is set to add to the group.\n";
        return false;
      }
      grpItem->setValue(modifyGroup);
    }

    // lanuch the group operation
    if (!modMgr->startOperation(grpOp))
    {
      std::cout << "Could not start operator: \"" << s_internal_groupOpName << "\" for session"
                << " \"" << sessPtr->name() << "\""
                << " (" << sessPtr->sessionId() << ")\n";
      return false;
    }
    return true;
  }
  return false;
}

void pqCMBContextMenuHelper::setRepresentationType(
  pqDataRepresentation* repr, const QString& repType)
{
  if (!repr)
    return;
  pqSMAdaptor::setEnumerationProperty(repr->getProxy()->GetProperty("Representation"), repType);
  repr->getProxy()->UpdateVTKObjects();
  repr->renderViewEventually();
}

void pqCMBContextMenuHelper::getAllEntityIds(
  pqSMTKModelInfo* minfo, smtk::model::ManagerPtr modelMan, smtk::common::UUIDs& entids)
{
  smtk::model::Model modelEntity(modelMan, minfo->Info->GetModelUUID());
  entids.insert(modelEntity.entity());

  smtk::model::CellEntities cellents = modelEntity.cells();
  for (smtk::model::CellEntities::const_iterator it = cellents.begin(); it != cellents.end(); ++it)
    entids.insert((*it).entity());

  smtk::model::Groups groups = modelEntity.groups();
  for (smtk::model::Groups::iterator git = groups.begin(); git != groups.end(); ++git)
    entids.insert((*git).entity());

  std::map<smtk::common::UUID, vtkIdType>::const_iterator it =
    minfo->Info->GetUUID2BlockIdMap().begin();
  for (; it != minfo->Info->GetUUID2BlockIdMap().end(); ++it)
    entids.insert(it->first);
}

void pqCMBContextMenuHelper::getAllMeshSets(
  pqSMTKMeshInfo* minfo, smtk::mesh::ManagerPtr meshMgr, smtk::mesh::MeshSets& meshes)
{
  smtk::mesh::CollectionPtr c = meshMgr->collection(minfo->Info->GetMeshCollectionID());
  meshes.insert(c->meshes());

  smtk::mesh::MeshSet dim3Group = c->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet dim2Group = c->meshes(smtk::mesh::Dims2);
  smtk::mesh::MeshSet dim1Group = c->meshes(smtk::mesh::Dims1);
  if (!dim3Group.is_empty())
    meshes.insert(dim3Group);
  if (!dim2Group.is_empty())
    meshes.insert(dim2Group);
  if (!dim1Group.is_empty())
    meshes.insert(dim1Group);

  std::map<smtk::mesh::MeshSet, vtkIdType>::const_iterator it =
    minfo->Info->GetMesh2BlockIdMap().begin();
  for (; it != minfo->Info->GetMesh2BlockIdMap().end(); ++it)
  {
    meshes.insert(it->first);
  }
}

void pqCMBContextMenuHelper::updateColorForAuxiliary(pqDataRepresentation* rep, const QColor& color)
{
  if (!rep)
  {
    return;
  }
  if (color.isValid())
  {
    double rgba[4];
    rgba[0] = color.redF();
    rgba[1] = color.greenF();
    rgba[2] = color.blueF();
    rgba[3] = color.alphaF() > 0. ? color.alphaF() : 1.;

    vtkSMPropertyHelper(rep->getProxy(), "DiffuseColor").Set(rgba, 3);
    vtkSMPropertyHelper(rep->getProxy(), "AmbientColor").Set(rgba, 3);
    vtkSMPropertyHelper(rep->getProxy(), "Opacity").Set(rgba[3]);
    rep->getProxy()->UpdateVTKObjects();
  }
}

void pqCMBContextMenuHelper::updateVisibilityForAuxiliary(pqDataRepresentation* rep, bool visible)
{
  if (rep)
  {
    rep->setVisible(visible);
  }
}
