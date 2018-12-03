find_package(ZLIB)
if (ZLIB_FOUND)
    set_source_files_properties(Writer.cc Reader.cc PROPERTIES
    COMPILE_DEFINITIONS "HAVE_ZLIB_H")
endif()

option(3DZIP_RANGECOD_EXTRAFAST "Use faster but less efficient range codec"
    false)
if (3DZIP_RANGECOD_EXTRAFAST)
    set_source_files_properties(RangeCod.cc PROPERTIES COMPILE_DEFINITIONS
    "EXTRAFAST")
endif()

include_directories(../libhds)

add_library(3dzip ActiveList.cc ActiveStack.cc RangeCod.cc ArCode.cc
    BitStream.cc Compressor.cc CompressorFacade.cc ManifoldCompressorFacade.cc
    ManifoldUncompressorFacade.cc Mesh.cc Reader.cc Symbol.cc Uncompressor.cc
    UncompressorFacade.cc Writer.cc)
target_link_libraries(3dzip hds ${ZLIB_LIBRARIES})
