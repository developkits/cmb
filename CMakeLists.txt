cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0025)
  #setup policy rules for CMake 3.0 while we have a minimum required of 2.8.X
  #both 42 and 25 were introduced in the same cmake version
  cmake_policy(SET CMP0025 NEW)  # Clang is now Clang and Apple Clang
  cmake_policy(SET CMP0042 OLD)  # Disable @rpath on Apple
endif()

#If the user/superbuild hasn't explicitly stated what c++ standard to use
#require C++11
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  set(CMAKE_CXX_EXTENSIONS FALSE)
endif()


project(ConceptualModelBuilder)
#this property needs to be internal as we don't want the user to ever see or
#modify it. This is used to allow people to continue using the Source folder
#as the root of the project
set(ConceptualModelBuilder_Project_Name_Set True
    CACHE INTERNAL "State we have a project name specified")


option(BUILD_TESTING "Build CMB Testing" ON)
if(BUILD_TESTING)
  enable_testing()
  include(CTest)
endif()

# Add options for performing code coverage tests.
include(${CMAKE_CURRENT_SOURCE_DIR}/Source/CMake/CMBCoverage.cmake)

# Do not report some warnings from generated code to the dashboard:
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/CMake)

add_subdirectory(ThirdParty)
add_subdirectory(Source)



option(CMB_ENABLE_DOCUMENTATION
  "Include targets Sphinx-generated documentation" OFF)
if (CMB_ENABLE_DOCUMENTATION)
  find_package(Sphinx)


  add_subdirectory(Documentation)
endif()
