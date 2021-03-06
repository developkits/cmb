//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLIDARReaderManager - .
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLIDARReaderManager_h
#define __pqCMBLIDARReaderManager_h

#include "cmbSystemConfig.h"
#include "vtkWeakPointer.h"
#include <QList>
#include <QMap>
#include <QObject>

class pqCMBPointsBuilderMainWindowCore;
class pqObjectBuilder;
class pqPipelineSource;
class pqCMBLIDARPieceTable;
class pqCMBLIDARPieceObject;
class vtkSMSourceProxy;
class pqCMBModifierArcManager;

class pqCMBLIDARReaderManager : public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARReaderManager(pqCMBPointsBuilderMainWindowCore*, pqObjectBuilder*);
  ~pqCMBLIDARReaderManager() override;

  bool userRequestsOrigin();
  bool setOrigin();

  bool userRequestsDoubleData();
  bool setOutputDataTypeToDouble();
  void convertFromLatLongToXYZ(int convertFromLatLong);

  void limitReadToBounds(bool limitRead);
  void setReadBounds(QList<QVariant>& values);

  void getDataBounds(double bounds[6]);

  int importData(const char* filename, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);

  vtkSMSourceProxy* getReaderSourceProxy(const char* filename);

  void setReaderTransform(vtkSMSourceProxy* readerProxy, pqCMBLIDARPieceObject* dataObj);
  void setReaderTransformData(vtkSMSourceProxy* readerProxy, bool transformData);
  void clearTransform(vtkSMSourceProxy* readerProxy);

  void readPieces(
    pqPipelineSource* reader, QList<QVariant>& pieces, QList<pqPipelineSource*>& pdSources);

  void updatePieces(const char* filename, QList<pqCMBLIDARPieceObject*>& pieces, bool forceRead,
    pqCMBLIDARPieceTable* table, bool clipData, double* clipBounds);

  bool getSourcesForOutput(bool atDisplayRatio, QList<pqCMBLIDARPieceObject*>& Pieces,
    QList<pqPipelineSource*>& outputSources, bool forceUpdate = false);

  void destroyTemporarySources();

  vtkIdType scanFileNumberOfPoints(pqPipelineSource* reader);
  vtkIdType scanLASPiecesInfo(pqPipelineSource* reader);
  void destroyAllReaders();
  QString getFileTitle() const;
  bool isValidFile(const char* filename);
  bool isFileLoaded(QString& filename);
  bool hasReaderSources() { return this->ReaderSourceMap.count() > 0; }
  vtkIdType scanTotalNumPointsInfo(const QStringList& files, pqPipelineSource* reader = NULL);
  QMap<QString, pqPipelineSource*>& readerSourceMap() { return this->ReaderSourceMap; }
  vtkSMSourceProxy* activeReader();
  QList<pqCMBLIDARPieceObject*> getFilePieceObjects(
    const char* filename, QList<pqCMBLIDARPieceObject*>& sourcePieces);

protected:
  int computeApproximateRepresentingFloatDigits(double min, double max);
  int importLIDARData(const char* filenmame, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);
  int importLASData(const char* filenmame, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);
  int importDEMData(const char* filenmame, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);
  int importGDALData(const char* filenmame, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);
  int importVTPData(const char* filenmame, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&, bool showElevation);
  vtkIdType getPieceNumPointsInfo(const char* filename, QList<vtkIdType>& pieceInfo);

  int readData(vtkSMSourceProxy* readerProxy, QList<QVariant>& pieces);
  void updateLIDARPieces(const char* filename, QList<pqCMBLIDARPieceObject*>& pieces,
    bool forceRead, pqCMBLIDARPieceTable* table, bool clipData, double* clipBounds);
  void updateLASPieces(const char* filename, QList<pqCMBLIDARPieceObject*>& pieces, bool forceRead,
    pqCMBLIDARPieceTable* table, bool clipData, double* clipBounds);
  bool getSourcesForOutputLIDAR(pqPipelineSource* reader, bool atDisplayRatio,
    QList<pqCMBLIDARPieceObject*>& Pieces, QList<pqPipelineSource*>& outputSources,
    bool forceUpdate);
  bool getSourcesForOutputLAS(pqPipelineSource* reader, bool atDisplayRatio,
    QList<pqCMBLIDARPieceObject*>& Pieces, QList<pqPipelineSource*>& outputSources,
    bool forceUpdate);

  pqCMBPointsBuilderMainWindowCore* Core;
  pqObjectBuilder* Builder;
  // Map for <Filename, readersource >
  QMap<QString, pqPipelineSource*> ReaderSourceMap;
  vtkWeakPointer<vtkSMSourceProxy> ActiveReader;

  double CurrentReaderBounds[6];
  QList<pqPipelineSource*> TemporaryPDSources;
  QList<pqPipelineSource*> TemporaryThresholdFilters;
  QList<pqPipelineSource*> TemporaryContourFilters;

  bool ChangeOrigin;
  double Origin[3];

private:
  struct LASPieceInfo
  {
    std::string ClassificationName;
    vtkIdType NumberOfPointsInClassification;
  };
  std::map<unsigned char, LASPieceInfo> CurrentLASPieces;
  double DEMTransformForZOrigin[2];
  double DEMZRotationAngle;
};

#endif /* __pqCMBLIDARReaderManager_h */
