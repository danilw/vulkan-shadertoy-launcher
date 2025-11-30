cmake_minimum_required(VERSION 3.11)

project("shadertoy_launcher")

OPTION(USE_WAYLAND_LINUX "use Wayland for Linux" OFF)

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    MESSAGE(FATAL_ERROR "ERROR: Vulkan not found")
endif(NOT Vulkan_FOUND)
set(Vulkan_LIBRARY)

set(Wayland_client "")
IF (NOT WIN32)
if(USE_WAYLAND_LINUX)
  
  find_program(PKG_CONFIG pkg-config)
  if (NOT PKG_CONFIG)
    MESSAGE(FATAL_ERROR "pkg-config binary not found")
  endif ()
	find_program(WAYLAND_SCANNER wayland-scanner)
	if (NOT WAYLAND_SCANNER)
		MESSAGE(FATAL_ERROR "wayland-scanner binary not found")
	endif ()
  set(Wayland_client "${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.c")
  set(WAYLAND_INCLUDE_DIR "/usr/include/wayland")
  MESSAGE(STATUS "  Wayland include path ${WAYLAND_INCLUDE_DIR}")
  include_directories(${WAYLAND_INCLUDE_DIR})
  execute_process(COMMAND ${PKG_CONFIG} --variable=pkgdatadir wayland-protocols OUTPUT_VARIABLE protocol_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
			COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c)
	include_directories(${CMAKE_BINARY_DIR})
endif ()
endif ()

if (Vulkan_FOUND)
    MESSAGE(STATUS "  [Found Vulkan library]")
    MESSAGE(STATUS "  Vulkan include directory:" ${Vulkan_INCLUDE_DIR})
    FOREACH(item ${Vulkan_LIBRARIES})
        MESSAGE(STATUS "  using lib: " ${item})
    ENDFOREACH()
    include_directories(${Vulkan_INCLUDE_DIR})
endif()

set(SOURCES
    "main.c" 
    "../vk_utils/vk_utils.c"
    "../vk_utils/vk_error_print.c"
    "../vk_utils/vk_render_helper.c"
    ${Wayland_client}
)

add_executable(
        Vk_shadertoy
        ${SOURCES} 
)

add_custom_command(TARGET Vk_shadertoy POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Vk_shadertoy>/shaders)
add_custom_command(TARGET Vk_shadertoy POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:Vk_shadertoy>/textures)

if(("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC"))
  add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DUSE_stb_image)
  target_link_libraries(Vk_shadertoy ${Vulkan_LIBRARY})
elseif(("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") AND ("${CMAKE_SYSTEM}" MATCHES "Linux"))
  if(USE_WAYLAND_LINUX)
    add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR -DUSE_stb_image -O2 -s -fdata-sections -ffunction-sections -Wl,--gc-sections)
    target_link_libraries(Vk_shadertoy m wayland-client ${Vulkan_LIBRARY} )
  else()
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR -DUSE_stb_image -O2 -s -fdata-sections -ffunction-sections -Wl,--gc-sections)
    target_link_libraries(Vk_shadertoy m xcb xcb-keysyms ${Vulkan_LIBRARY} )
  endif()
else()
  message(WARNING "Unknown compiler '${CMAKE_C_COMPILER_ID}'!")
endif()

