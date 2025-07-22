# CMake generated Testfile for 
# Source directory: /home/kel/Desktop/malloy/test
# Build directory: /home/kel/Desktop/malloy/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[doctest]=] "/home/kel/Desktop/malloy/build/bin/malloy-tests")
set_tests_properties([=[doctest]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kel/Desktop/malloy/test/CMakeLists.txt;32;add_test;/home/kel/Desktop/malloy/test/CMakeLists.txt;0;")
subdirs("test_suites")
