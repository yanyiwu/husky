SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(threadpoolserver threadpoolserver.cpp)
TARGET_LINK_LIBRARIES(threadpoolserver pthread)
