# The CLI, melevel. It records the coredcc commit it was built against.
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/extern/coredcc"
  OUTPUT_VARIABLE _coredcc_commit RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT _result STREQUAL "0")
  set(_coredcc_commit "unknown")
endif()

add_executable(melevel main.cc)
target_link_libraries(melevel PRIVATE ue3 coredcc_base)
target_compile_definitions(melevel PRIVATE MELEVEL_COREDCC_COMMIT="${_coredcc_commit}")
melevel_target_flags(melevel)
