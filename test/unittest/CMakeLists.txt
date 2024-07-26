SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test/lib)

SET(GTEST_ROOT_DIR googletest)
INCLUDE_DIRECTORIES(${GTEST_ROOT_DIR} ${GTEST_ROOT_DIR}/include)
ADD_DEFINITIONS(-DGTEST_USE_OWN_TR1_TUPLE=1)
ADD_LIBRARY(gtest STATIC ${GTEST_ROOT_DIR}/src/gtest-all.cc)
TARGET_LINK_LIBRARIES(gtest pthread)
ADD_EXECUTABLE(test.run gtest_main.cc http_req_info_ut.cc)
ADD_EXECUTABLE(server.test gtest_main.cc thread_pool_server_ut.cc)
TARGET_LINK_LIBRARIES(gtest pthread)
TARGET_LINK_LIBRARIES(test.run gtest pthread)
TARGET_LINK_LIBRARIES(server.test gtest pthread)

