project(3DZ_DEBUG)
find_package(Qt4 COMPONENTS QtXml REQUIRED)
include(${QT_USE_FILE})
find_path(QGLVIEWER_INCLUDE_DIR NAMES QGLViewer/qglviewer.h)
include_directories(${3DZ_DEBUG_BINARY_DIR} ${QGLVIEWER_INCLUDE_DIR})
find_library(QGLVIEWER_LIBRARY NAMES qglviewer-qt4 QGLViewer QGLViewer2)
QT4_WRAP_UI(UI_SRC MainWindow.ui)
QT4_WRAP_CPP(MOC_SRC MainWindow.hpp)
add_executable(3dz-debug 3dz-debug.cpp ${UI_SRC} ${MOC_SRC} MainWindow.hpp MainWindow.cpp)
target_link_libraries(3dz-debug ${GUI_LIBRARIES} ${QGLVIEWER_LIBRARY} 3dzip hds)
