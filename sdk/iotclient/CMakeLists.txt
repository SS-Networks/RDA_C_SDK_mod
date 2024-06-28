cmake_minimum_required(VERSION 2.8.4)

project("iotclient" C)
message(STATUS "## BUILD START ${PROJECT_NAME}")
message(STATUS "## PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "## PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "## CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message(STATUS "## CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}/common"
					"${CMAKE_SOURCE_DIR}/protocol"
					"${CMAKE_SOURCE_DIR}/message"
					"${CMAKE_SOURCE_DIR}/manager"
					"${CMAKE_SOURCE_DIR}/security"
					"${CMAKE_SOURCE_DIR}/security/aes"
					"/usr/local/include"
)

set(sub_libs 
             utils 
			 protocol 
			 message
			 manager
			 security
)

foreach(lib_item ${sub_libs}) 
	set(lib_list ${lib_list} $<TARGET_OBJECTS:${lib_item}>) 
endforeach()

set(src_files_iotclient
  		IotClient queue
)

add_library(iotclient STATIC ${lib_list} ${src_files_iotclient})

install(TARGETS iotclient
		RUNTIME DESTINATION ${SDK_OUTPUT_DIR}/lib/ocp
		LIBRARY DESTINATION ${SDK_OUTPUT_DIR}/lib/ocp
        ARCHIVE DESTINATION ${SDK_OUTPUT_DIR}/lib/ocp
)

install(FILES IotClient.h
    DESTINATION ${SDK_OUTPUT_DIR}/include/ocp
)

