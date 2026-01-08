
if (ZI_VULKAN_ENABLED)
	add_subdirectory(vma)
	add_subdirectory(volk)
	add_subdirectory(vulkan)
endif ()

if (ZI_DESKTOP)
	add_subdirectory(glfw)
endif ()
