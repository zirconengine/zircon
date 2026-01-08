file(GLOB_RECURSE ZIRCON_RUNTIME_SOURCES
				${CMAKE_CURRENT_SOURCE_DIR}/*.h
				${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(zi-runtime STATIC ${ZIRCON_RUNTIME_SOURCES})
target_include_directories(zi-runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(zi-runtime PRIVATE
				glfw
				vma
				volk
				vulkan-sdk
)