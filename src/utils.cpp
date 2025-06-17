#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

void secureDelete(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file for secure delete: " << path << std::endl;
        return;
    }

    std::streamsize size = file.tellg();
    file.close();

    std::ofstream file_to_wipe(path, std::ios::binary | std::ios::out);
    if (!file_to_wipe.is_open()) {
        std::cerr << "Error opening file for writing: " << path << std::endl;
        return;
    }

    // Overwrite with random data 3 times
    for (int i = 0; i < 3; ++i) {
        std::vector<char> buffer(size);
        for (std::streamsize j = 0; j < size; ++j) {
            buffer[j] = rand();
        }
        file_to_wipe.seekp(0);
        file_to_wipe.write(buffer.data(), size);
        file_to_wipe.flush();
    }

    // Overwrite with zeros
    std::vector<char> zeros(size, 0);
    file_to_wipe.seekp(0);
    file_to_wipe.write(zeros.data(), size);
    file_to_wipe.flush();
    file_to_wipe.close();

    // Finally, remove the file
    if (remove(path.c_str()) != 0) {
        perror("Error deleting file");
    } else {
        std::cout << "Securely deleted file: " << path << std::endl;
    }
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string generateRandomId(size_t length) {
    const std::string charset = "abcdef0123456789";
    std::string result;
    result.resize(length);
    for (size_t i = 0; i < length; i++) {
        result[i] = charset[rand() % charset.length()];
    }
    return result;
}
