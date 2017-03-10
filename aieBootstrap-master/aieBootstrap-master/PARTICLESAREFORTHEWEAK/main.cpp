#include "PARTICLESAREFORTHEWEAKApp.h"

int main() {
	
	auto app = new PARTICLESAREFORTHEWEAKApp();
	app->run("AIE", 1280, 720, false);
	delete app;

	return 0;
}