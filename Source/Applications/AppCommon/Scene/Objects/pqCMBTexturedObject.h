//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBTexturedObject - represents a 3D Scene object that can be textured mapped.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBTexturedObject_h
#define __pqCMBTexturedObject_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class  CMBAPPCOMMON_EXPORT pqCMBTexturedObject : public pqCMBSceneObjectBase
{
public:

  pqCMBTexturedObject();
  pqCMBTexturedObject(pqPipelineSource *source,
                         pqRenderView *view,
                         pqServer *server);
  virtual ~pqCMBTexturedObject();

  /// Returns the Bounds of the data. - Returns the output of the Bathymetry
  virtual void getDataBounds(double bounds[6]) const;
  virtual pqPipelineSource * getSelectionSource() const;
  virtual void setSelectionInput(vtkSMSourceProxy *selectionInput);
  virtual vtkSMSourceProxy *getSelectionInput() const;

  void showElevation(bool flag);
  void toggleElevation()
  { this->showElevation(!this->ShowElevation);}
  bool showingElevation() const
  { return this->ShowElevation;}
  const double* getRegistrationPoints()
  { return this->RegistrationPoints;}

  bool hasTexture() const
    { return (this->NumberOfRegistrationPoints > 0);}

  const QString & getTextureFileName()
    { return this->TextureFileName;}

  int getNumberOfRegistrationPoints() const
    { return this->NumberOfRegistrationPoints;}

  void getRegistrationPointPair(int i, double xy[2], double st[2]) const;

  double getTextureIntensityAtPoint(double pt[3]);
  void applyBathymetry(pqCMBSceneObjectBase* bathymetrySource,
    double elevationRadious, bool useHighLimit=false,
    double eleHigh=0.0, bool useLowLimit=false, double eleLow=0.0);
  void unApplyBathymetry();
  pqCMBSceneObjectBase* getBathymetrySource();
  double getBathymetryElevationRadious()
    {return this->ElevationRadious;}
  pqServer* getTextureRegistrationServer(void);

  void setTextureMap(const char *filename, int numberOfRegistrationPoints,
      double *points);
  void unsetTextureMap();

protected:
  QPointer<pqPipelineSource> RegisterTextureFilter;
  QPointer<pqPipelineSource> TexturePointIntensityFilter;
  QPointer<pqPipelineSource> ElevationFilter;
  QPointer<pqPipelineSource> TextureImageSource;
  QString TextureFileName;
  int NumberOfRegistrationPoints;
  double RegistrationPoints[12];

  QPointer<pqPipelineSource> BathymetryFilter;
  pqCMBSceneObjectBase* BathymetrySource;
  double ElevationRadious;
  bool ShowElevation;
  void prepTexturedObject(pqServer *server, pqRenderView *view);
  virtual void duplicateInternals(pqCMBSceneObjectBase *obj);

};

#endif /* __pqCMBTexturedObject_h */
