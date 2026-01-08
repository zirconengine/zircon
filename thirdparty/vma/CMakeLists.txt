add_library(vma STATIC vk_mem_alloc.cpp)
target_include_directories(vma PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(vma PRIVATE vulkan-sdk)
