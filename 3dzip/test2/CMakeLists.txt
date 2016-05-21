qt4_wrap_cpp(MOC_SRC MainWindow.hpp GLWidget.hpp GLViewer.hpp)
qt4_wrap_ui(UI_SRC Main.ui)
add_executable(multimesh GLViewer.hpp GLWidget.hpp MainWindow.hpp Scene.hpp
    main.cpp Scene.cpp GLViewer.cpp MainWindow.cpp GLWidget.cpp ${MOC_SRC}
    ${UI_SRC})
include_directories(${CMAKE_CURRENT_BINARY_DIR} .)
target_link_libraries(multimesh 3dzip hds ${GUI_LIBRARIES})
