#include "InteractiveApp.hpp"
#include <iostream>

int main() {
    try {
        InteractiveApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
