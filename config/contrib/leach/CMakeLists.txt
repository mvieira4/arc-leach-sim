check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/leach-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME leach
    SOURCE_FILES model/leach-app.cc
                 model/arc-leach-app.cc
                 model/alt-route-app.cc

                 model/leach-sink-app.cc
                 model/arc-leach-sink-app.cc
                 model/alt-route-sink-app.cc

                 model/info-id-tag.cc
                 model/leach-tag.cc
                 model/leach-tag-list.cc
                 model/seq-num-tag.cc
                 model/timestamp-tag.cc

                 helper/leach-helper.cc
                 helper/arc-leach-helper.cc
                 helper/alt-route-helper.cc

                 helper/leach-sink-helper.cc
                 helper/arc-leach-sink-helper.cc
                 helper/alt-route-sink-helper.cc

    HEADER_FILES model/leach-app.h
                 model/arc-leach-app.h
                 model/alt-route-app.h

                 model/leach-sink-app.h
                 model/leach-tag-list.h
                 model/arc-leach-sink-app.h
                 model/alt-route-sink-app.h

                 model/info-id-tag.h
                 model/leach-tag.h
                 model/seq-num-tag.h
                 model/timestamp-tag.h

                 helper/leach-helper.h
                 helper/arc-leach-helper.h
                 helper/alt-route-helper.h

                 helper/leach-sink-helper.h
                 helper/arc-leach-sink-helper.h
                 helper/alt-route-sink-helper.h

    LIBRARIES_TO_LINK ${libcore}
    TEST_SOURCES test/leach-test-suite.cc
                 ${examples_as_tests_sources}
)

