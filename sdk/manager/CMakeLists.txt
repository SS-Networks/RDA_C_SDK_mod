cmake_minimum_required(VERSION 2.8.4)

project("manager" C)
message(STATUS "## BUILD START ${PROJECT_NAME}")
message(STATUS "## PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "## PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "## CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message(STATUS "## CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}/common"
					"${CMAKE_SOURCE_DIR}/protocol"
					"${CMAKE_SOURCE_DIR}/message"
					"${CMAKE_SOURCE_DIR}/manager"
)

set(src_files_manager
  		OCPManager
		OCPThingManager
        OCPAPIManager
)

#add_library(manager STATIC ${src_files_manager})
add_library(manager OBJECT ${src_files_manager})

install(FILES OCPManager.h
    DESTINATION ${SDK_OUTPUT_DIR}/include/ocp
)

