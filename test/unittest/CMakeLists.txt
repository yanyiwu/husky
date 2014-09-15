SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test/lib)

SET(GTEST_ROOT_DIR gtest-1.6.0)

INCLUDE_DIRECTORIES(${GTEST_ROOT_DIR} ${GTEST_ROOT_DIR}/include ${PROJECT_SOURCE_DIR}/include)
ADD_LIBRARY(gtest STATIC ${GTEST_ROOT_DIR}/src/gtest-all.cc)
ADD_EXECUTABLE(test.run gtest_main.cpp THttpReqInfo.cpp)
ADD_EXECUTABLE(server.test gtest_main.cpp TThreadPoolServer.cpp)
TARGET_LINK_LIBRARIES(gtest pthread)
TARGET_LINK_LIBRARIES(test.run gtest pthread)
TARGET_LINK_LIBRARIES(server.test gtest pthread)

