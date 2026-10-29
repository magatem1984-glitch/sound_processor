#include "sound_processor/application.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    constexpr int kErrorCode = 1;
    constexpr int kUnknownErrorCode = 2;

    try {
        Application application;
        application.configure();
        return application.start(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return kErrorCode;
    } catch (...) {
        std::cerr << "error: unknown exception\n";
        return kUnknownErrorCode;
    }
}
