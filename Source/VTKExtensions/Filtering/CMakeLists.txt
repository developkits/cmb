# VXL is needed.
find_package(VXL)
find_package(OpenCV REQUIRED)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

set(CMB_Filtering_SRC
    vtkAddCellDataFilter.cxx
    vtkAddMeshDataArrayFilter.cxx
    vtkArcDepressFilter.cxx
    vtkCMBApplyBathymetryFilter.cxx
    vtkClipPolygons.cxx
    vtkClosestPointFilter.cxx
    vtkCMBArcAutoConnectOperator.cxx
    vtkCMBArcCreateOperator.cxx
    vtkCMBArcDeleteOperator.cxx
    vtkCMBArcGrowOperator.cxx
    vtkCMBArcMergeArcsOperator.cxx
    vtkCMBArcPickPointOperator.cxx
    vtkCMBArcPointGlyphingFilter.cxx
    vtkCMBArcSplitOnIndexOperator.cxx
    vtkCMBArcSplitOnPositionOperator.cxx
    vtkCMBArcUpdateOperator.cxx
    vtkCMBClassifyPointsFilter.cxx
    vtkCMBConeCellClassifier.cxx
    vtkCMBContourGroupFilter.cxx
    vtkCMBExtractContours.cxx
    vtkCMBGlyphPointSource.cxx
    vtkCMBImageClassFilter.cxx
    vtkCMBMedialAxisFilter.cxx
    vtkCMBMeshConeSelector.cxx
    vtkCMBMeshContourSelector.cxx
    vtkCMBMeshSelectionConverter.cxx
    vtkCMBSmoothMeshFilter.cxx
    vtkCMBSubArcModifyOperator.cxx
    vtkGMSMeshSelectionRegionFilter.cxx
    vtkIdentifyNonManifoldPts.cxx
    vtkLIDARElevationFilter.cxx
    vtkPointThresholdFilter.cxx
    vtkRegisterPlanarTextureMap.cxx
    vtkTerrainExtractionFilter.cxx
    vtkTexturePointIntensityFilter.cxx

    vtkCMBGrabCutFilter.cxx
    vtkCMBWatershedFilter.cxx
)

set(vtkCMBFiltering_NO_HeaderTest 1)
vtk_module_library(vtkCMBFiltering ${CMB_Filtering_SRC})

target_include_directories(vtkCMBFiltering
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../LIDAR
            PRIVATE ${VXL_RPL_INCLUDE_DIR}
                    ${VXL_VCL_INCLUDE_DIR}
                    ${VXL_CORE_INCLUDE_DIR}
                    ${SMTK_INCLUDE_DIRS}/extension/vtk/filter/
                    ${Boost_INCLUDE_DIRS}
            )

target_link_libraries(vtkCMBFiltering
            PRIVATE
              rtvl ${OpenCV_LIBS}
              rgtl
              vnl
              vtkIOImage
              vtkSMTKReaderExt # for vtkLIDARReader
              vtkSMTKFilterExt
              vtkFiltersSources
            )
