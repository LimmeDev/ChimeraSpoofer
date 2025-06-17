#ifndef SPOOFER_HPP
#define SPOOFER_HPP

#include <string>
#include <vector>

class Spoofer {
public:
    Spoofer();
    void setMacAddress(const std::string& interface, const std::string& macAddress);
    void spoofMachineId(const std::string& newId);
    void restoreMachineId();
    void spoofHostname(const std::string& newHostname);
    void restoreHostname();
    void spoofBootId();
    void restoreBootId();
    void spoofAll();
    void restoreAll();
    void spoofDiskSerials();
    void restoreDiskSerials();
    void spoofDiskUUID(const std::string& device, const std::string& newUUID);
    void restoreDiskUUID(const std::string& device);
    void spoofDiskLabel(const std::string& device, const std::string& newLabel);
    void restoreDiskLabel(const std::string& device);
    void spoofOsRelease();
    void restoreOsRelease();
    void spoofSshFingerprint();
    void restoreSshFingerprint();
    void spoofIpIdFingerprint();
    void restoreIpIdFingerprint();
    void spoofTimestamps(const std::string& path, const std::string& timestamp);
    void restoreTimestamps(const std::string& path);

#ifdef __linux__
private:
    const std::string machineIdPath = "/etc/machine-id";
    const std::string hostnamePath = "/etc/hostname";
    const std::string bootIdPath = "/proc/sys/kernel/random/boot_id";
    const std::string osReleasePath = "/etc/os-release";
    const std::string sshConfigPath = "/etc/ssh/ssh_config";
    const std::string diskBackupFile = "disk_serials.bak";

    void createBackupDir();
    void backup(const std::string& identifier, const std::string& value);
    std::string readFromBackup(const std::string& identifier);
    void restore(const std::string& identifier, const std::string& path);
    std::string readFromFile(const std::string& path);
    void writeToFile(const std::string& path, const std::string& value);
    bool fileExists(const std::string& path);
    void spoofMacAddress(const std::string& interface, const std::string& newMac);
#endif

    std::string backupDir;
};

#endif // SPOOFER_HPP 