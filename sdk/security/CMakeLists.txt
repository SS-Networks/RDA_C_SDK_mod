cmake_minimum_required(VERSION 2.8.4)

project("security" C)
message(STATUS "## BUILD START ${PROJECT_NAME}")
message(STATUS "## PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "## PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "## CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message(STATUS "## CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")

include_directories("${CMAKE_SOURCE_DIR}/common"
					"${PROJECT_SOURCE_DIR}/aes/tiny-aes"
					"${PROJECT_SOURCE_DIR}/base64"
)

set(src_files
				OCPSecurity
  				${PROJECT_SOURCE_DIR}/aes/OCPSecurityAES
  				${PROJECT_SOURCE_DIR}/aes/tiny-aes/aes_ocp
  				${PROJECT_SOURCE_DIR}/base64/decode
  				${PROJECT_SOURCE_DIR}/base64/encode
)

add_library(security OBJECT ${src_files})

