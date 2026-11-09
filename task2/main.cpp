#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Hello from app\n";
    std::cout << "argc = " << argc << "\n";
    for (int i = 1; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }
    return 0;
}