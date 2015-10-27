cmake_minimum_required(VERSION 2.8.11)

project(T1T2FiberAnalyzer)

#snippet taken from: http://www.guyrutenberg.com/2014/01/05/enabling-c11-c0x-in-cmake/
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set( EXTRA_FLAG "-std=c++11" )
elseif(COMPILER_SUPPORTS_CXX0X)
  set( EXTRA_FLAG "-std=c++0x" )
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
if( EXTRA_FLAG )
  set( CMAKE_CXX_FLAGS_LIST ${CMAKE_CXX_FLAGS} )
  separate_arguments(CMAKE_CXX_FLAGS_LIST)
  get_property( helpstring CACHE CMAKE_CXX_FLAGS PROPERTY HELPSTRING )
  list(FIND CMAKE_CXX_FLAGS_LIST ${EXTRA_FLAG} found_flag )
  if( found_flag LESS 0 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAG}" CACHE STRING "${helpstring}" FORCE )
  endif()
endif()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)

option( CSV_IO_NO_THREAD "Do not use threads for CSV_IO" TRUE )
if( CSV_IO_NO_THREAD )
  add_definitions(-DCSV_IO_NO_THREAD)
endif()

find_package(Qt5Widgets REQUIRED)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})
include(${GenerateCLP_USE_FILE})

find_package(QtToCppXML REQUIRED)
include(${QtToCppXML_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Resources)

set(srcheaders
include/atlasmodel.h
include/csvbrowsemodel.h
include/fibertractmodel.h
include/csvbrowser.h
include/T1T2FiberAnalyzer.h
include/errorreporter.h
)

set(srcfiles
tfatool.cpp
errorreporter.cpp
T1T2FiberAnalyzer.cpp
csvbrowser.cpp
csvbrowsemodel.cpp
fibertractmodel.cpp
atlasmodel.cpp
)

set(srcUI
csvbrowser.ui
mainwindow.ui
)

qt5_wrap_cpp(QT5_HEADERS_MOC ${srcheaders})
qt5_wrap_ui(UI_FILES ${srcUI})

QtToCppXML( SOURCES srcfiles NAME T1T2FiberAnalyzer UIXML_FILE mainwindow.ui)

list( APPEND
srcfiles
${QT5_HEADERS_MOC}
${UI_FILES}
)

SEMMacroBuildCLI(
  NAME T1T2FiberAnalyzerTool
  #EXECUTABLE_ONLY
  ADDITIONAL_SRCS ${srcfiles}
  TARGET_LIBRARIES ${Qt5Widgets_LIBRARIES} 
  INCLUDE_DIRECTORIES ${Qt5Widgets_INCLUDE_DIRS} 
)

#add_executable(T1T2FiberAnalyzer ${srcfiles})
#target_link_libraries(T1T2FiberAnalyzer Qt5::Widgets)
