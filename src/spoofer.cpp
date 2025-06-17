#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cstring>
#include <sstream>

const std::string MACHINE_ID_PATH = "/etc/machine-id";
const std::string BACKUP_PATH = "/var/lib/chimera/original_machine-id";

// ... (previous spoofer code for machine-id)
// This will be a simplified version for brevity in this context.

void backup_file(const std::string& original_path, const std::string& backup_path) {
    std::ifstream src(original_path, std::ios::binary);
    std::ofstream dst(backup_path, std::ios::binary);
    dst << src.rdbuf();
}

void restore_file(const std::string& original_path, const std::string& backup_path) {
    std::ifstream src(backup_path, std::ios::binary);
    if (!src) {
        std::cerr << "Error: Backup file not found." << std::endl;
        return;
    }
    std::ofstream dst(original_path, std::ios::binary);
    dst << src.rdbuf();
}

void spoof_machine_id(const std::string& new_id) {
    backup_file(MACHINE_ID_PATH, BACKUP_PATH);
    std::ofstream ofs(MACHINE_ID_PATH, std::ios::trunc);
    ofs << new_id;
}


int main(int argc, char* argv[]) {
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root." << std::endl;
        return 1;
    }

    if (argc < 2) {
        std::cerr << "Usage: spoofer <command> [options]" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    if (command == "spoof-machine-id") {
        if (argc < 3) {
            std::cerr << "Usage: spoofer spoof-machine-id <new-id>" << std::endl;
            return 1;
        }
        spoof_machine_id(argv[2]);
        std::cout << "Spoofed machine-id to " << argv[2] << std::endl;
    } else if (command == "restore") {
        restore_file(MACHINE_ID_PATH, BACKUP_PATH);
        std::cout << "Restored original machine-id." << std::endl;
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
} 