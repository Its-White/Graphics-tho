#include "ParticleApp.h"

int main() {
	
	auto app = new ParticleApp();
	app->run("AIE", 1280, 720, false);
	delete app;

	return 0;
}