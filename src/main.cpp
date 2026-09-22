#include "Engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    GameEngine engine;
    if (!engine.initialize()) {
        std::cerr << "Critical Error: Engine failed to initialize!" << std::endl;
        return 1;
    }

    engine.run();
    engine.shutdown();

    std::cout << "[SUCCESS] Production Module Safely Terminated." << std::endl;
    return 0;
}
