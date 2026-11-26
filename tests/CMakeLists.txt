melevel_test(test_ue3_result SOURCES ue3/test_result.cc DEPS ue3)

# The CLI. With a PASS_REGULAR_EXPRESSION CTest ignores the exit code, so the failing
# exit of an unknown command is its own test.
add_test(NAME cli_version COMMAND melevel version)
set_tests_properties(cli_version PROPERTIES
  PASS_REGULAR_EXPRESSION "^\\{\"tool\":\"melevel\",\"version\":\"[0-9.]+\",\"coredcc\":\"[0-9a-f]+\"\\}")
add_test(NAME cli_unknown_command COMMAND melevel nope)
set_tests_properties(cli_unknown_command PROPERTIES
  PASS_REGULAR_EXPRESSION "^\\{\"error\":\\{\"message\":\"unknown command 'nope'")
add_test(NAME cli_unknown_command_exit COMMAND melevel nope)
set_tests_properties(cli_unknown_command_exit PROPERTIES WILL_FAIL TRUE)
