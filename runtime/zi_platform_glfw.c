#include "zi_log.h"

#if ZI_DESKTOP
#include <GLFW/glfw3.h>

#include "zi_app.h"

void zi_app_init();
void zi_app_loop();
void zi_app_terminate();

void zi_app_set_app_running(u8 p_is_running);
void zi_bootstrap(ZiAppSettings* settings);

i32 zi_platform_run(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	zi_app_init();

	ZiAppSettings settings = {0};
	zi_bootstrap(&settings);

	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	GLFWwindow* main_window = glfwCreateWindow(800, 600, settings.title, NULL, NULL);

	while (zi_app_is_running()) {
		glfwPollEvents();

		if (glfwWindowShouldClose(main_window)) {
			zi_app_set_app_running(ZI_FALSE);
		}

		zi_app_loop();
	}

	glfwDestroyWindow(main_window);

	zi_app_terminate();

	glfwTerminate();

	return 0;
}

#endif
