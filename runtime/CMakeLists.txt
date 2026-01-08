file(GLOB_RECURSE ZIRCON_RUNTIME_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(zi-runtime STATIC ${ZIRCON_RUNTIME_SOURCES})
target_include_directories(zi-runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if (ZI_DESKTOP)
	target_link_libraries(zi-runtime PRIVATE
			glfw
	)
	target_compile_definitions(zi-runtime PUBLIC ZI_DESKTOP=1)
endif ()

if (EMSCRIPTEN)
	target_link_options(zi-runtime PUBLIC
			-sASYNCIFY
			-sWASM=1
			-sALLOW_MEMORY_GROWTH=1
	)

	target_compile_options(zi-runtime PUBLIC "--use-port=emdawnwebgpu")
	target_link_options(zi-runtime PUBLIC "--use-port=emdawnwebgpu")
endif ()

if (ZI_VULKAN_ENABLED)
	target_link_libraries(zi-runtime PRIVATE
			vma
			volk
			vulkan-sdk
	)
	target_compile_definitions(zi-runtime PUBLIC ZI_VULKAN_ENABLED)
endif ()