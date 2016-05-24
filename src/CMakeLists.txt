add_library(filesystem8
    #codecvt_error_category
    operations
    path
    #path_traits
    portability
    #unique_path
    #utf8_codecvt_facet
    #windows_file_codecvt
)

# please fix and enable this if you can test it on hpux machine
# if(COMPILER_IS_GNUXX AND CMAKE_SYSTEM MATCHES HPUX)
#    target_compiler_definitions(filesystem8 PRIVATE
#        _INCLUDE_STDC__SOURCE_199901)
#endif()

include(GenerateExportHeader)
generate_export_header(filesystem8
    BASE_NAME filesystem8
    EXPORT_FILE_NAME include/filesystem8/export.h)

target_include_directories(filesystem8 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS filesystem8 EXPORT filesystem8-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT filesystem8-targets FILE filesystem8-config.cmake
    DESTINATION lib/cmake/filesystem8
    NAMESPACE ::)

install(DIRECTORY include DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/filesystem8/export.h
    DESTINATION include/filesystem8)
