cmake_minimum_required(VERSION 2.8)

project(yariv)

set(SOURCES
	"main.c" 
)

add_executable(
		yariv_pack
		${SOURCES}
)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/Wall /WX /wd4514)
  #target_link_libraries(yariv_pack)
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-Wall -Wextra)
  #target_link_libraries(yariv_pack)
else()
  message(WARNING "Unknown compiler '${CMAKE_C_COMPILER_ID}'!")
endif()
