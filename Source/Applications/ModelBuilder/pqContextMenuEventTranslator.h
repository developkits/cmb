//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __pqContextMenuEventTranslator_h
#define __pqContextMenuEventTranslator_h

#include "cmbSystemConfig.h"
#include "pqWidgetEventTranslator.h"

class QAction;

/**
Translates low-level Qt events into high-level ParaView events that can be recorded as test cases.

\sa pqEventTranslator
*/

class pqContextMenuEventTranslator : public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  pqContextMenuEventTranslator(QObject* p = 0);
  ~pqContextMenuEventTranslator() override;

  bool translateEvent(QObject* Object, QEvent* Event, bool& Error) override;

private:
  pqContextMenuEventTranslator(const pqContextMenuEventTranslator&);
  pqContextMenuEventTranslator& operator=(const pqContextMenuEventTranslator&);
};

#endif // !_pqContextMenuEventTranslator_h
