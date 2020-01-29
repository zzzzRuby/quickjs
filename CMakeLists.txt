cmake_minimum_required(VERSION 3.0)
project(quickjs C)

set(CMAKE_C_STANDARD 11)

option(QUICKJS_BUILD_TOOL "build qjs, qjsc and jscompress" on)
option(QUICKJS_BUILD_TEST "build run-test262" on)
option(QUICKJS_ENABLE_BIGNUM "enable bignum" on)

file(READ ${CMAKE_CURRENT_LIST_DIR}/VERSION QUICKJS_VERSION)
string(STRIP "${QUICKJS_VERSION}" QUICKJS_VERSION)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-D_GNU_SOURCE)
endif()
add_definitions("-DCONFIG_VERSION=\"${QUICKJS_VERSION}\"")
if (QUICKJS_ENABLE_BIGNUM)
    add_definitions(-DCONFIG_BIGNUM)
endif()

find_package(Threads)
add_library(quickjs STATIC
    quickjs.c quickjs.h quickjs-opcode.h quickjs-atom.h
    cutils.c cutils.h list.h
    libunicode.c libunicode.h libunicode-table.h
    libregexp.c libregexp.h libregexp-opcode.h)
target_include_directories(quickjs
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/msvc/main
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if (QUICKJS_ENABLE_BIGNUM)
    target_sources(quickjs PRIVATE libbf.c libbf.h)
endif()
if (TARGET Threads::Threads)
    target_link_libraries(quickjs PRIVATE Threads::Threads)
endif()
if (NOT MSVC)
    target_link_libraries(quickjs PRIVATE m dl)
else()
    target_sources(quickjs PRIVATE
        msvc/main/msvc.c)
endif()

if (QUICKJS_BUILD_TEST OR QUICKJS_BUILD_TOOL)
    add_library(quickjs-libc
        quickjs-libc.c quickjs-libc.h)
    target_link_libraries(quickjs-libc PRIVATE quickjs)
    if (MSVC)
        target_sources(quickjs-libc PRIVATE 
            msvc/libc/src/getopt.c
            msvc/libc/src/libc.c
            msvc/libc/src/tool.c)
        target_include_directories(quickjs-libc PUBLIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/msvc/libc/include
            ${CMAKE_CURRENT_SOURCE_DIR}/msvc/main)
        set_source_files_properties(quickjs-libc.c PROPERTIES COMPILE_DEFINITIONS QUICKJS_USE_IN_LIBC)
        target_compile_definitions(quickjs-libc INTERFACE QUICKJS_USE_IN_TOOL)
    endif()
endif()

if (QUICKJS_BUILD_TOOL)
    add_executable(qjsc qjsc.c)
    target_link_libraries(qjsc PRIVATE quickjs-libc)
    add_executable(jscompress jscompress.c)
    target_link_libraries(jscompress PRIVATE quickjs-libc)

    add_executable(qjs qjs.c)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/repl.c
        COMMAND qjsc -c -o ${CMAKE_CURRENT_BINARY_DIR}/repl.c -m ${CMAKE_CURRENT_SOURCE_DIR}/repl.js
        DEPENDS qjsc ${CMAKE_CURRENT_SOURCE_DIR}/repl.js)
    target_sources(qjs PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/repl.c)
    if (QUICKJS_ENABLE_BIGNUM)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qjscalc.c
            COMMAND qjsc -fbignum -c -o ${CMAKE_CURRENT_BINARY_DIR}/qjscalc.c ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js
            DEPENDS qjsc ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js)
        target_sources(qjs PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/qjscalc.c)
    endif()
    target_link_libraries(qjs PRIVATE quickjs-libc)
endif()

if (QUICKJS_BUILD_TEST)
    add_executable(run-test262 run-test262.c)
    target_link_libraries(run-test262 PRIVATE quickjs-libc)

    add_custom_command(TARGET run-test262 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/test262.conf ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/test262o.conf ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/test262_errors.txt ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/test262o_errors.txt ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test262 ${CMAKE_CURRENT_BINARY_DIR}/test262)
endif()

# a bug of clang-cl
# see https://bugs.llvm.org/show_bug.cgi?id=25305
if (MSVC AND "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "AMD64")
        set(arch "x86_64")
    endif()
    get_filename_component(CLANG_PATH "${CMAKE_C_COMPILER}" DIRECTORY)
    get_filename_component(builtin_lib "${CLANG_PATH}/../lib/clang/${CMAKE_C_COMPILER_VERSION}/lib/windows/clang_rt.builtins-${arch}.lib" ABSOLUTE)
    target_link_libraries(quickjs PRIVATE ${builtin_lib})
endif()