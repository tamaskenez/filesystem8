add_library(filesystem
    #codecvt_error_category
    operations
    path
    #path_traits
    portability
    unique_path
    #utf8_codecvt_facet
    #windows_file_codecvt
)

if(COMPILER_IS_GNUXX AND CMAKE_SYSTEM MATCHES HPUX)
    target_compiler_definitions(filesystem PRIVATE
        _INCLUDE_STDC__SOURCE_199901)
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(filesystem PUBLIC BOOST_FILESYSTEM_DYN_LINK=1)
else()
    target_compile_definitions(filesystem PUBLIC BOOST_FILESYSTEM_STATIC_LINK=1)
endif()

include(GenerateExportHeader)
generate_export_header(filesystem
    BASE_NAME boost_filesystem
    EXPORT_FILE_NAME include/boost/filesystem/export.h)

target_include_directories(filesystem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS filesystem EXPORT filesystem-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT filesystem-targets FILE filesystem-config.cmake
    DESTINATION lib/cmake/filesystem)

install(DIRECTORY include DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/boost/filesystem/export.h
    DESTINATION include/boost/filesystem)
