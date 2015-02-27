/*=========================================================================

  Program:   CMB
  Module:    qtRemusVolumeMesherSelector.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME qtRemusVolumeMesherSelector
// .SECTION Description
// Simply displays all the different volume meshers that are available to the
// user.
// .SECTION Caveats

#ifndef __qtRemusVolumeMesherSelector_h
#define __qtRemusVolumeMesherSelector_h

#include <QDialog>

//Don't let QMOC see remus headers that include boost headers
//or bad things happen
#ifndef Q_MOC_RUN
  #include <remus/client/ServerConnection.h>
  #include <remus/proto/JobRequirements.h>
#endif

class QListWidgetItem;

class qtRemusVolumeMesherSelector : public QDialog
{
  Q_OBJECT
public:
  qtRemusVolumeMesherSelector(QString serverEndpoint,
                           QWidget* parent);

  //returns true if the user has selected a mesher
  bool chooseMesher();

  bool useLegacyMesher() const;

  QString mesherName() const { return ActiveMesher; }
  const remus::proto::JobRequirements& mesherData() const { return ActiveMesherData; }

public slots:
  void mesherChanged(QListWidgetItem* item);

private:
  QString ActiveMesher;
  remus::proto::JobRequirements ActiveMesherData;
  remus::client::ServerConnection Connection;

};

#endif
