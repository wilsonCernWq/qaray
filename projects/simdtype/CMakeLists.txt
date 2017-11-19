option(ENABLE_SIMDTYPE_PROJECT "Build SIMD Type Project" OFF)
if (ENABLE_SIMDTYPE_PROJECT)
    #
    # get ptoject info
    get_filename_component(PID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(TOLOWER ${PID} PID)
    #
    # include sources
    file(GLOB_RECURSE MATH_SRC "${CMAKE_CURRENT_SOURCE_DIR}/math/*")
    file(GLOB MAIN "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    include_directories(${CMAKE_CURRENT_LIST_DIR})
    #
    #
    set(ALL_LIBS "")
    macro(qw_add_library name)
        set(lib_name ${name}_${PID})
        set(src_list "")
        set(lnk_list "")
        string(TOUPPER ${name}_SRC src_list)
        message(STATUS "[ ${lib_name} ]")
        foreach (f ${${src_list}})
            file(RELATIVE_PATH rf ${CMAKE_CURRENT_SOURCE_DIR} ${f})
            message(STATUS "   * ${rf}")
        endforeach ()
        foreach (l ${ARGN})
            set(lnk_list ${lnk_list} ${l}_${PID})
        endforeach ()
        message(STATUS "link to ${lnk_list}")
        add_library(${name}_${PID} ${${src_list}})
        target_link_libraries(${name}_${PID} ${lnk_list} ${COMMON_LIBS})
        set(ALL_LIBS ${ALL_LIBS} ${name}_${PID})
    endmacro()
    #
    # add library
    qw_add_library(math)
    #
    # add executable
    add_executable(test_${PID} ${MAIN})
    target_link_libraries(test_${PID} ${ALL_LIBS} ${COMMON_LIBS})
endif (ENABLE_SIMDTYPE_PROJECT)
