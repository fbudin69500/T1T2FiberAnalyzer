cmake_minimum_required(VERSION 2.8.11)

project(T1T2FiberAnalyzer)

add_definitions(-std=c++11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)

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
