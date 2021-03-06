//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef qtCMBProfilePointFunctionModifier_h_
#define qtCMBProfilePointFunctionModifier_h_

#include <QPointer>
#include <QWidget>
#include <vector>

#include "pqCMBModifierArc.h"
#include "pqGeneralTransferFunctionWidget.h"

class Ui_qtCMBProfileWedgeFunction;
class cmbProfileWedgeFunction;
class cmbProfileWedgeFunctionParameters;

class qtCMBProfileWedgeFunctionWidget : public QWidget
{
  Q_OBJECT
public:
  qtCMBProfileWedgeFunctionWidget(QWidget* parent, cmbProfileWedgeFunction* function);
  ~qtCMBProfileWedgeFunctionWidget() override;
  void setRelative(bool);
protected slots:
  void setLeftSlope(double);
  void setRightSlope(double);
  void setBaseWidth(double);
  void setDepth(double);
  void setSymmetry(bool);
  void setClamp(bool);
  void setMode(int);
  void weightSplineBox(bool);
  void render();

protected:
  Ui_qtCMBProfileWedgeFunction* UI;
  cmbProfileWedgeFunction* function;
  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;

  void setUp();
  void setWeightFunction();
};

#endif
