#include "zi_app.h"

#include "zi_graphics.h"
#include "zi_log.h"


static u8 is_running = ZI_FALSE;

void zi_graphics_init(ZiGraphicsBackend backend);
void zi_graphics_terminate();

void zi_app_init() {
	zi_graphics_init(0);
	is_running = ZI_TRUE;
}

void zi_app_loop() {
	//zi_log_info("app is running");
}

void zi_app_terminate() {
	zi_graphics_terminate();
	is_running = ZI_FALSE;
}

u8 zi_app_is_running() {
	return is_running;
}

void zi_app_set_app_running(u8 p_is_running) {
	is_running = p_is_running;
}
