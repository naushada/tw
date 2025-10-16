# CMake generated Testfile for 
# Source directory: /home/mnahmed/tw/rpc_server/libevent_adapter/test
# Build directory: /home/mnahmed/tw/rpc_server/libevent_adapter/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/home/mnahmed/tw/rpc_server/libevent_adapter/build/test/remote_app_test[1]_include.cmake")
add_test(remote_app_gtests "remote_app_test")
set_tests_properties(remote_app_gtests PROPERTIES  _BACKTRACE_TRIPLES "/home/mnahmed/tw/rpc_server/libevent_adapter/test/CMakeLists.txt;55;add_test;/home/mnahmed/tw/rpc_server/libevent_adapter/test/CMakeLists.txt;0;")
subdirs("../_deps/googletest-build")
