#include "Post_ProcessingApp.h"

int main() {
	
	auto app = new Post_ProcessingApp();
	app->run("AIE", 1280, 720, false);
	delete app;

	return 0;
}