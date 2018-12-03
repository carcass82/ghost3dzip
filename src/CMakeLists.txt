find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(EIGEN2 REQUIRED eigen2)
    include_directories(${EIGEN2_INCLUDE_DIRS})
endif()
find_package(LAPACK REQUIRED)

# metis option
option(SPECCOMP_METIS "Use MeTiS to split big meshes" false)
if (SPECCOMP_METIS)
    add_subdirectory(${SPECCOMP_SOURCE_DIR}/src/metis)
    include_directories(${METIS_SOURCE_DIR})
    set(METIS_LIBRARY metis)

    add_definitions(-DHAVE_METIS_H=1)
    set(METIS_SRCS metispart.cpp)
endif()

# give option for choosing huffman or aritmetic coding
option(SPECCOMP_HUFFMAN "Use Huffman coding for compression" false)
if(SPECCOMP_HUFFMAN)
    add_definitions(-DUSE_HUFFMAN_CODER)
endif()

set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -fpermissive")

option(SPECCOMP_OPENMP "Include OpenMP compiler directives where applicable" false)
if(SPECCOMP_OPENMP)
    find_package(OpenMP REQUIRED)
    add_definitions(-DUSE_OPENMP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -fpermissive")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set_target_properties(PROPERTIES LINK_FLAGS "${OpenMP_CXX_FLAGS}")
endif()

option(SPECCOMP_CUDA "Use NVidia CUDA devices to compute in parallel"  false)
if(SPECCOMP_CUDA)
    find_package(CUDA REQUIRED)
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -Xcompiler -fpermissive)
    set(CUDA_SRCS eigensolver.cu)
    add_definitions(-DUSE_CUDA)
endif()

option(SPECCOMP_PROFILE "Generate gprof profiling infos" false)
if(SPECCOMP_PROFILE)
    add_definitions(-pg)
    set(CMAKE_EXE_LINKER_FLAGS -pg)
endif()

option(SPECCOMP_GUI "Build GUI application" false)
if(SPECCOMP_GUI)
    find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXml REQUIRED)
    find_package(QGLViewer REQUIRED)
    find_package(OpenGL REQUIRED)

    add_definitions(${QT_DEFINITIONS})
    include(${QT_USE_FILE})

    include_directories(${QGLVIEWER_INCLUDE_DIR})

    qt4_wrap_cpp(GUI_MOC_SRC gui.h)
    qt4_wrap_ui(GUI_UI_SRC gui.ui)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(GUI_SRCS gui.cpp ${GUI_UI_SRC} ${GUI_MOC_SRC})
endif()

set(SPECCOMP_SRCS encoder.cpp decoder.cpp eigensolver.cpp utils.cpp huffman.c ${CTRIE_SRCS} ${HUFFMAN_SRCS} ${METIS_SRCS} ${CUDA_SRCS})
set(SPECCOMP_LINKS ${LAPACK_LIBRARIES} ${METIS_LIBRARY} 3dzip hds)

if(SPECCOMP_CUDA)
    cuda_add_executable(specencode main.cpp ${SPECCOMP_SRCS})
else()
    add_executable(specencode main.cpp ${SPECCOMP_SRCS})
endif()
target_link_libraries(specencode ${SPECCOMP_LINKS})

if(SPECCOMP_GUI)
    if(SPECCOMP_CUDA)
        cuda_add_executable(specgui ${GUI_SRCS} ${SPECCOMP_SRCS})
    else()
        add_executable(specgui ${GUI_SRCS} ${SPECCOMP_SRCS})
    endif()
    target_link_libraries(specgui ${OPENGL_gl_LIBRARY} ${QT_LIBRARIES}  ${QGLVIEWER_LIBRARY} ${SPECCOMP_LINKS})
endif()

add_custom_command(
    TARGET specencode
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SPECCOMP_BINARY_DIR}/src/specencode${CMAKE_EXECUTABLE_SUFFIX}
    ${SPECCOMP_BINARY_DIR}/src/specdecode${CMAKE_EXECUTABLE_SUFFIX}
)
