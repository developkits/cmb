//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBModelBuilderOptions_h
#define _pqCMBModelBuilderOptions_h

#include "cmbSystemConfig.h"
#include "qtCMBOptionsContainer.h"

/// options container for pages of model builder and sim builder options
class pqCMBModelBuilderOptions : public qtCMBOptionsContainer
{
  Q_OBJECT

public:
  // Get the global instace for the pqCMBModelBuilderOptions.
  static pqCMBModelBuilderOptions* instance();

  pqCMBModelBuilderOptions(QWidget* parent = 0);
  ~pqCMBModelBuilderOptions() override;

  // set the current page
  void setPage(const QString& page) override;
  // return a list of strings for pages we have
  QStringList getPageList() override;

  // apply the changes
  void applyChanges() override;
  // reset the changes
  void resetChanges() override;

  // tell qtCMBOptionsDialog that we want an apply button
  bool isApplyUsed() const override { return true; }

  // Get the options
  std::string defaultSimBuilderTemplateDirectory();
  std::string default3DModelFaceColorMode();
  std::string default2DModelFaceColorMode();
  std::string default2DModelEdgeColorMode();
  // Color is stored on QEntityItemModel so smtk does not need to tether Qt and PV
  QColor defaultFaceColor();
  QColor defaultEdgeColor();
  QColor defaultVertexColor();
  bool sessionCentricModeling();
  bool createDefaultSessionModel();
  bool autoSwitchCameraManipulator();
  bool askBeforeDiscardingChanges();
signals:
  void updateEntityColor();

protected slots:
  void chooseSimBuilderTemplateDirectory();

  void setAskBeforeDiscardingChanges(bool doAsk);
  // As a convenience for pqCMBModelBuilderMainWindowCore's popup, provide a variant of the above.
  void setDoNotAskBeforeDiscardingChanges(bool dontAsk)
  {
    this->setAskBeforeDiscardingChanges(!dontAsk);
  }

private:
  class pqInternal;
  pqInternal* Internal;
  static pqCMBModelBuilderOptions* Instance;
};

#endif
