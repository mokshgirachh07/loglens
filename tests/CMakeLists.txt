include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(loglens_tests
  test_parser.cpp
  test_analyzer.cpp
  test_anomaly.cpp
)
target_link_libraries(loglens_tests PRIVATE loglens_core GTest::gtest_main)

add_test(NAME loglens_tests COMMAND loglens_tests)