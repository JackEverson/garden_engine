#include <print>
#include "Engine.hpp"

#include "IntroScene.hpp"

int main() {

	std::string name = "Sushi Jumper";
	GardenEngine engine(name, false, 1920, 1080);
	// GardenEngine engine(name, true, 1920, 1080);

	float fps = 60.0f;
	//float fps = 144.0f;

	auto scene = std::make_unique<IntroScene>();
	int engine_test_result = engine.Start(std::move(scene), fps);

	std::println("Engine Exit code: {}", engine_test_result);

	return engine_test_result;
}
