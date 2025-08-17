#include <thread>
#include <chrono>

// Forward declare user functions
void setup();
void loop();

int main() {
    setup();  // call user setup once

    while (true) {
        loop();  // repeatedly call user loop
    }

    return 0;
}
