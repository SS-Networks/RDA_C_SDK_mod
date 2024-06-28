cmake_minimum_required(VERSION 2.8.4)
project("protocol" C)
message(STATUS "## BUILD START ${PROJECT_NAME}")
message(STATUS "## PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "## PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "## CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message(STATUS "## CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}/common"
					/usr/local/include
)

if(PROTOCOL MATCHES ${protocol_mqtt})
message(STATUS "PROTOCOL MATCHES ${protocol_mqtt}")
	set(src_files 
	              	OCPProtocolMqtt
	)
endif()

if(PROTOCOL MATCHES ${protocol_curl})
message(STATUS "PROTOCOL MATCHES ${protocol_curl}")
	set(src_files 
					${src_files}
	              	OCPProtocolCurl 
	)
endif()

if(NOT PROTOCOL OR PROTOCOL MATCHES ${protocol_all})
message(STATUS "NOT PROTOCOL OR PROTOCOL MATCHES ${protocol_all}")
	set(src_files 
					OCPProtocolMqtt
	              	OCPProtocolCurl 
	)
endif()

add_library(protocol OBJECT ${src_files})