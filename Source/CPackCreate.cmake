# If the cmake version includes cpack, use it
IF(NOT WIN32 AND NOT APPLE)
  SET(CPACK_SET_DESTDIR TRUE)
ENDIF()
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMB Suite of Tools")
SET(CPACK_PACKAGE_VENDOR "Kitware")
SET(CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "${CMB_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${CMB_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${CMB_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY
  "CMBSuite ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME
  "cmb-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
IF(NOT DEFINED CPACK_SYSTEM_NAME)
  SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
ENDIF(NOT DEFINED CPACK_SYSTEM_NAME)
IF(${CPACK_SYSTEM_NAME} MATCHES Windows)
  IF(CMAKE_CL_64)
    SET(CPACK_SYSTEM_NAME win64-${CMAKE_SYSTEM_PROCESSOR})
  ELSE(CMAKE_CL_64)
    SET(CPACK_SYSTEM_NAME win32-${CMAKE_SYSTEM_PROCESSOR})
  ENDIF(CMAKE_CL_64)
ENDIF(${CPACK_SYSTEM_NAME} MATCHES Windows)
IF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
  SET(CPACK_PACKAGE_FILE_NAME
    "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME}")
ENDIF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
SET(CPACK_PACKAGE_CONTACT "bill.hoffman@kitware.com")
IF(WIN32)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\ModelBuilder.exe")
  SET(CPACK_PACKAGE_EXECUTABLES
      "GeologyBuilder" "Geology Builder"
      "MeshViewer" "Mesh Viewer"
      "ModelBuilder" "Model Builder"
      "paraview" "ParaView"
      "PointsBuilder" "Points Builder"
      "ProjectManager" "Project Manager"
      "SceneBuilder" "Scene Builder"
    )
  SET(CPACK_CREATE_DESKTOP_LINK_prototype 1)
  SET(CPACK_NSIS_DISPLAY_NAME
    "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  SET(CPACK_NSIS_HELP_LINK "http://www.kitware.com")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.kitware.com")
  SET(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
  SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(WIN32)

IF(APPLE)
  SET(CPACK_GENERATOR "DragNDrop")
ENDIF(APPLE)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/CMake/CPackOptions.cmake.in"
  "${ConceptualModelBuilder_BINARY_DIR}/CPackOptions.cmake" @ONLY)
SET(CPACK_PROJECT_CONFIG_FILE
  "${ConceptualModelBuilder_BINARY_DIR}/CPackOptions.cmake")

INCLUDE(CPack)
