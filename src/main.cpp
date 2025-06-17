#include <iostream>
#include <memory>
#include <limits> // Required for std::numeric_limits
#include <unistd.h> // for geteuid
#include <linux/limits.h> // for PATH_MAX
#include "platform_factory.hpp"
#include "spoofer.hpp"
#include "trace_cleaner.hpp"
#include "utils.hpp"
#include <cstring>

void printMenu() {
    std::cout << "\n--- Chimera Spoofer - Interactive Menu ---\n";
    std::cout << "------------------------------------------\n";
    std::cout << " 1) Spoof All Identifiers\n";
    std::cout << " 2) Restore All Identifiers\n";
    std::cout << "------------------------------------------\n";
    std::cout << " 3) Spoof Hostname\n";
    std::cout << " 4) Spoof Machine ID\n";
    std::cout << " 5) Spoof OS Release\n";
    std::cout << " 6) Spoof SSH Fingerprint\n";
    std::cout << " 7) Spoof Network Fingerprint (Port Range)\n";
    std::cout << " 8) Spoof File Timestamp\n";
    std::cout << "------------------------------------------\n";
    std::cout << " 9) Restore Hostname\n";
    std::cout << "10) Restore Machine ID\n";
    std::cout << "11) Restore OS Release\n";
    std::cout << "12) Restore SSH Fingerprint\n";
    std::cout << "13) Restore Network Fingerprint (Port Range)\n";
    std::cout << "14) Restore File Timestamp\n";
    std::cout << "------------------------------------------\n";
    std::cout << "15) Securely Delete File\n";
    std::cout << "16) Clean Traces and Exit\n";
    std::cout << "------------------------------------------\n";
    std::cout << " 0) Exit\n";
    std::cout << "------------------------------------------\n";
    std::cout << "Enter your choice: ";
}

int main(int argc, char* argv[]) {
    if (geteuid() != 0) {
        std::cerr << "Error: This program must be run as root." << std::endl;
        return 1;
    }

    auto spoofer = PlatformFactory::createSpoofer();
    if (!spoofer) {
        std::cerr << "Error: Failed to create spoofer for this platform." << std::endl;
        return 1;
    }
    
    if (argc > 1 && strcmp(argv[1], "--spoof-all-persistent") == 0) {
        spoofer->spoofAll();
        return 0;
    }

    int choice = -1;
    while (choice != 0) {
        printMenu();
        std::cin >> choice;

        switch (choice) {
            case 1:
                spoofer->spoofAll();
                break;
            case 2:
                spoofer->restoreAll();
                break;
            case 3:
                spoofer->spoofHostname("chimera-" + generateRandomId(6));
                break;
            case 4:
                spoofer->spoofMachineId(generateRandomId(32));
                break;
            case 5:
                spoofer->spoofOsRelease();
                break;
            case 6:
                spoofer->spoofSshFingerprint();
                break;
            case 7:
                spoofer->spoofIpIdFingerprint();
                break;
            case 8: {
                std::string path, timestamp;
                std::cout << "Enter file path: ";
                std::cin >> path;
                std::cout << "Enter timestamp (e.g., '2000-01-01 12:00:00'): ";
                std::cin.ignore(); // Consume the newline from previous input
                std::getline(std::cin, timestamp);
                spoofer->spoofTimestamps(path, timestamp);
                break;
            }
            case 9:
                spoofer->restoreHostname();
                break;
            case 10:
                spoofer->restoreMachineId();
                break;
            case 11:
                spoofer->restoreOsRelease();
                break;
            case 12:
                spoofer->restoreSshFingerprint();
                break;
            case 13:
                spoofer->restoreIpIdFingerprint();
                break;
            case 14: {
                std::string path;
                std::cout << "Enter file path: ";
                std::cin >> path;
                spoofer->restoreTimestamps(path);
                break;
            }
            case 15: {
                std::string path;
                std::cout << "Enter file path to securely delete: ";
                std::cin >> path;
                secureDelete(path);
                break;
            }
            case 16: {
                char exePath[PATH_MAX];
                ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
                if (len != -1) {
                    exePath[len] = '\0';
                    TraceCleaner cleaner(".chimera_backup/", exePath);
                    cleaner.cleanAllTraces();
                    std::cout << "Self-destruct sequence initiated. Exiting now." << std::endl;
                    return 0;
                } else {
                    std::cerr << "Error: Could not determine executable path. Cannot self-destruct." << std::endl;
                }
                break;
            }
            case 0:
                std::cout << "Exiting Chimera Spoofer." << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
                // Clear the error flag and ignore the rest of the line.
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
        }
    }

    return 0;
} 