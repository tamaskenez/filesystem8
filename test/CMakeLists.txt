unset(CMAKE_CXX_VISIBILITY_PRESET)
unset(CMAKE_VISIBILITY_INLINES_HIDDEN)

foreach(a
       convenience_test
#       macro_default_test
       odr1_test
       odr2_test
       deprecated_test
       fstream_test
       large_file_support_test
       locale_info
       operations_test
       operations_unit_test
       path_test
       path_unit_test
       relative_test
       ../example/simple_ls
       ../example/file_status)

    get_filename_component(F ${a} NAME)
    set(t test_${F})
    add_executable(${t} ${a})
    target_link_libraries(${t} PRIVATE filesystem) 
    add_test(NAME ${t} COMMAND ${t})
endforeach()
