#ifndef PLATFORM_FACTORY_HPP
#define PLATFORM_FACTORY_HPP

#include "spoofer.hpp"
#include <memory>

class PlatformFactory {
public:
    static std::unique_ptr<Spoofer> createSpoofer() {
#ifdef __linux__
        return std::make_unique<Spoofer>();
#elif _WIN32
        // When Windows support is added, this will return the Windows-specific spoofer
        return nullptr; 
#else
        return nullptr; // Unsupported platform
#endif
    }
};

#endif // PLATFORM_FACTORY_HPP 