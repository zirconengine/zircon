file(GLOB_RECURSE ZIRCON_RUNTIME_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(zi-runtime STATIC ${ZIRCON_RUNTIME_SOURCES})
target_include_directories(zi-runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if (ZI_DESKTOP)
	target_link_libraries(zi-runtime PRIVATE
			glfw
	)
	target_compile_definitions(zi-runtime PUBLIC ZI_DESKTOP)
endif ()

if (ZI_VULKAN_ENABLED)
	target_link_libraries(zi-runtime PRIVATE
			vma
			volk
			vulkan-sdk
	)
	target_compile_definitions(zi-runtime PUBLIC ZI_VULKAN_ENABLED)
endif ()