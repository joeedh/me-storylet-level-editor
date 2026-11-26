cmake_minimum_required(VERSION 3.25)
project(me_level_tool C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MELEVEL_WERROR "Treat warnings in this project's targets as errors" ${PROJECT_IS_TOP_LEVEL})
option(MELEVEL_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/coredcc/cmake")
include(CoredccHelpers)
include(MelevelHelpers)

# No exceptions and no RTTI anywhere, coredcc and litestl included, so _HAS_EXCEPTIONS
# agrees in every translation unit. coredcc leaves this to its host (docs/embedding.md).
coredcc_disable_exceptions_and_rtti()

# Incremental linking fails at random with LNK1235 after many files change (coredcc's
# docs/debugging.md).
if(MSVC)
  foreach(_kind EXE SHARED MODULE)
    foreach(_config DEBUG RELWITHDEBINFO)
      string(APPEND CMAKE_${_kind}_LINKER_FLAGS_${_config} " /INCREMENTAL:NO")
    endforeach()
  endforeach()
endif()

# Only what our targets link gets built. coredcc's tests are off when it isn't the top.
add_subdirectory(extern/coredcc EXCLUDE_FROM_ALL SYSTEM)

add_subdirectory(source)

if(MELEVEL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
