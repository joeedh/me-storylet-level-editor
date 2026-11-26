# CMake helpers for this project's targets and tests.

# Warning and conformance flags for our own targets (not coredcc's or litestl's).
function(melevel_target_flags target)
  if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(${target} PRIVATE /W4 /utf-8)
    if(MELEVEL_WERROR)
      target_compile_options(${target} PRIVATE /WX)
    endif()
    # clang-cl rejects /Zc:preprocessor as an unused argument.
    target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    if(MELEVEL_WERROR)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# melevel_library(name SOURCES... [DEPS targets...])
function(melevel_library name)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEPS" ${ARGN})
  add_library(${name} STATIC ${ARG_SOURCES})
  target_include_directories(${name} PUBLIC "${PROJECT_SOURCE_DIR}/source")
  target_link_libraries(${name} PUBLIC ${ARG_DEPS})
  melevel_target_flags(${name})
endfunction()

# melevel_test(name SOURCES... [DEPS targets...])
# One executable per test, registered with CTest, on litestl's harness (test_init,
# test_assert, return test_end()) with coredcc's test support: its dtl stub, and a
# Windows hook that makes a failed assert exit instead of opening a dialog.
# LITESTL_TEST_NO_BREAK reports every failure instead of breaking on the first.
set(_MELEVEL_COREDCC "${CMAKE_CURRENT_LIST_DIR}/../extern/coredcc")
function(melevel_test name)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEPS" ${ARGN})
  add_executable(${name} ${ARG_SOURCES})
  target_link_libraries(${name} PRIVATE ${ARG_DEPS} util)
  melevel_target_flags(${name})
  target_include_directories(${name} SYSTEM PRIVATE
    "${_MELEVEL_COREDCC}/tests/support"
    "${_MELEVEL_COREDCC}/extern/litestl/tests")
  if(WIN32)
    target_sources(${name} PRIVATE "${_MELEVEL_COREDCC}/tests/support/no_dialogs.cc")
  endif()
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES ENVIRONMENT "LITESTL_TEST_NO_BREAK=1")
endfunction()
