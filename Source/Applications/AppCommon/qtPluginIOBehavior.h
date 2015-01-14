/*=========================================================================

   Program: ConceptualModelBuilder
   Module:    qtPluginIOBehavior.h

   Copyright (c) Kitware Inc.
   All rights reserved.

========================================================================*/
#ifndef __qtPluginIOBehavior_h
#define __qtPluginIOBehavior_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"

#include <QMap>
#include <QPair>

class vtkSMSession;
class vtkPVXMLElement;

/// This behavior handles readers and writers that are loaded from plugins.
class CMBAPPCOMMON_EXPORT qtPluginIOBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
  typedef QMap<QString, QPair<QString, QString> > FileExtMap;
public:
  qtPluginIOBehavior(QObject* parent=0);

  // Description:
  // Returns a formatted string with all supported file types.
  // An example returned string would look like:
  // "LAS Files (*.las);;Moab Files (*.moab)"
  const char* supportedFileTypes(vtkSMSession* session);

  // Description:
  // Returns the map <file_extension, <reader_group, reader_name> >
  // Example <brep, <sources, CmbMoabSolidReader> >
  FileExtMap fileExtensionMap(vtkSMSession* session);

  // Description:
  // check if the proxy is from a plugin by examining the hints
  static bool isPluginReader(vtkPVXMLElement* hints);

protected slots:
  void updateResources();

private:
  Q_DISABLE_COPY(qtPluginIOBehavior)
  class cmbInternals;
  cmbInternals* Internals;
};

#endif
