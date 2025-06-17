#ifdef __linux__

#include "spoofer.hpp"
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <random>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <regex>
#include <memory>
#include <array>
#include <algorithm>
#include "utils.hpp"
#include <cstdlib>
#include <ctime>
#include <stdexcept>

Spoofer::Spoofer() : backupDir(".chimera_backup/") {
    createBackupDir();
    srand(time(0));
}

std::string Spoofer::readFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

void Spoofer::writeToFile(const std::string& path, const std::string& value) {
    std::ofstream file(path);
    if (!file) {
        std::cerr << "Error: Could not open file for writing: " << path << std::endl;
        return;
    }
    file << value;
}

void Spoofer::spoofMachineId(const std::string& newId) {
    std::string original = readFromFile(machineIdPath);
    if (!original.empty()) {
        backup("machine_id", original);
    writeToFile(machineIdPath, newId);
        std::cout << "Machine ID spoofed to: " << newId << std::endl;
    }
}

void Spoofer::restoreMachineId() {
    restore("machine_id", machineIdPath);
    }

void Spoofer::spoofMacAddress(const std::string& interface, const std::string& newMac) {
    setMacAddress(interface, newMac);
}

void Spoofer::setMacAddress(const std::string& interface, const std::string& macAddress) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl(SIOCGIFFLAGS)");
        close(sock);
        return;
    }

    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl(SIOCSIFFLAGS) down");
        close(sock);
        return;
    }

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    sscanf(macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &ifr.ifr_hwaddr.sa_data[0], &ifr.ifr_hwaddr.sa_data[1],
           &ifr.ifr_hwaddr.sa_data[2], &ifr.ifr_hwaddr.sa_data[3],
           &ifr.ifr_hwaddr.sa_data[4], &ifr.ifr_hwaddr.sa_data[5]);

    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0) {
        perror("ioctl(SIOCSIFHWADDR)");
        // Try to bring the interface back up anyway
        ifr.ifr_flags |= IFF_UP;
        ioctl(sock, SIOCSIFFLAGS, &ifr);
        close(sock);
        return;
    }

    ifr.ifr_flags |= IFF_UP;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl(SIOCSIFFLAGS) up");
    }

    close(sock);

    std::cout << "Successfully changed MAC address of " << interface << " to " << macAddress << std::endl;
}

void Spoofer::spoofHostname(const std::string& newHostname) {
    std::string original = readFromFile(hostnamePath);
    if (!original.empty()) {
        backup("hostname", original);
        writeToFile(hostnamePath, newHostname);
        std::cout << "Hostname spoofed to: " << newHostname << std::endl;
    }
}

void Spoofer::restoreHostname() {
    restore("hostname", hostnamePath);
}

void Spoofer::spoofBootId() {
    std::string original = readFromFile(bootIdPath);
    if (!original.empty()) {
        backup("boot_id", original);
        std::string newBootId = generateRandomId(32);
        newBootId.insert(8, 1, '-');
        newBootId.insert(13, 1, '-');
        newBootId.insert(18, 1, '-');
        newBootId.insert(23, 1, '-');
        writeToFile(bootIdPath, newBootId);
        std::cout << "Boot ID spoofed to: " << newBootId << std::endl;
    }
}

void Spoofer::restoreBootId() {
    restore("boot_id", bootIdPath);
}

void Spoofer::spoofAll() {
    std::cout << "--- Starting Quick Spoof ---" << std::endl;
    spoofHostname("chimera-" + generateRandomId(6));
    spoofMachineId(generateRandomId(32));
    spoofBootId();
    spoofDiskSerials();
    spoofOsRelease();
    spoofSshFingerprint();
    spoofIpIdFingerprint();
    // Note: MAC spoofing requires interface and address, so it's not included in 'all' yet.
    std::cout << "--- Quick Spoof Finished ---" << std::endl;
}

void Spoofer::restoreAll() {
    std::cout << "--- Restoring All Backups ---" << std::endl;
    restoreHostname();
    restoreMachineId();
    restoreBootId();
    restoreDiskSerials();
    restoreOsRelease();
    restoreSshFingerprint();
    restoreIpIdFingerprint();
    // Clean up backup directory if empty
    if (opendir(backupDir.c_str()) == NULL) {
         rmdir(backupDir.c_str());
    }
    std::cout << "--- Restoration Finished ---" << std::endl;
}

void Spoofer::createBackupDir() {
    mkdir(backupDir.c_str(), 0700);
}

void Spoofer::backup(const std::string& identifier, const std::string& value) {
    if (!fileExists(backupDir + identifier)) {
        writeToFile(backupDir + identifier, value);
        std::cout << "Backed up original " << identifier << std::endl;
    }
}

std::string Spoofer::readFromBackup(const std::string& key) {
    return readFromFile(backupDir + key);
}

void Spoofer::restore(const std::string& identifier, const std::string& path) {
    std::string backupPath = backupDir + identifier;
    if (fileExists(backupPath)) {
        std::string originalValue = readFromFile(backupPath);
        if (!originalValue.empty()) {
            writeToFile(path, originalValue);
            remove(backupPath.c_str());
            std::cout << "Restored " << identifier << std::endl;
        }
    } else {
        std::cout << "No backup found for " << identifier << ", skipping." << std::endl;
    }
}

bool Spoofer::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

void Spoofer::spoofDiskSerials() {
    std::cout << "--- Spoofing Disk Serials ---" << std::endl;
    std::string json_output = exec("lsblk -J -o NAME,UUID,FSTYPE");

    // Backup original serials before changing them
    std::ofstream backup_file(backupDir + diskBackupFile);

    std::regex re("\"name\":\"(.*?)\".*?\"uuid\":\"(.*?)\".*?\"fstype\":\"(.*?)\"");
    std::smatch match;
    std::string::const_iterator search_start(json_output.cbegin());

    while (std::regex_search(search_start, json_output.cend(), match, re)) {
        std::string name = "/dev/" + match[1].str();
        std::string uuid = match[2].str();
        std::string fstype = match[3].str();

        if (uuid != "null" && !uuid.empty()) {
            std::cout << "Found " << name << " (" << fstype << ") with UUID: " << uuid << std::endl;
            
            // Write to backup
            backup_file << name << " " << uuid << " " << fstype << std::endl;

            // Generate new UUID
            std::string new_uuid = generateRandomId(32);
            new_uuid.insert(8, 1, '-');
            new_uuid.insert(13, 1, '-');
            new_uuid.insert(18, 1, '-');
            new_uuid.insert(23, 1, '-');
            
            std::string command;
            if (fstype.rfind("ext", 0) == 0) { // ext2, ext3, ext4
                command = "tune2fs -U " + new_uuid + " " + name;
            } else if (fstype == "xfs") {
                command = "xfs_admin -U " + new_uuid + " " + name;
            } else if (fstype == "btrfs") {
                 command = "btrfstune -U " + new_uuid + " " + name;
            } else if (fstype == "swap") {
                command = "swaplabel -U " + new_uuid + " " + name;
            }

            if (!command.empty()) {
                std::cout << "Executing: " << command << std::endl;
                exec(command.c_str());
            } else {
                std::cout << "Unsupported fstype for UUID change: " << fstype << std::endl;
            }
        }
        search_start = match.suffix().first;
    }
     backup_file.close();
    std::cout << "--- Disk Serials Spoofing Finished ---" << std::endl;
}

void Spoofer::restoreDiskSerials() {
    std::cout << "--- Restoring Disk Serials ---" << std::endl;
    std::ifstream backup_file(backupDir + diskBackupFile);
    if (!backup_file.is_open()) {
        std::cout << "No disk serial backup found, skipping." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(backup_file, line)) {
        std::istringstream iss(line);
        std::string name, uuid, fstype;
        if (!(iss >> name >> uuid >> fstype)) {
            continue; // error
        }

        std::cout << "Restoring " << name << " to UUID: " << uuid << std::endl;
        std::string command;
        if (fstype.rfind("ext", 0) == 0) {
            command = "tune2fs -U " + uuid + " " + name;
        } else if (fstype == "xfs") {
            command = "xfs_admin -U " + uuid + " " + name;
        } else if (fstype == "btrfs") {
             command = "btrfstune -U " + uuid + " " + name;
        } else if (fstype == "swap") {
            command = "swaplabel -U " + uuid + " " + name;
        }

        if (!command.empty()) {
            exec(command.c_str());
        }
    }
    backup_file.close();
    remove((backupDir + diskBackupFile).c_str());
    std::cout << "--- Disk Serials Restoration Finished ---" << std::endl;
}

void Spoofer::spoofOsRelease() {
    std::string original = readFromFile(osReleasePath);
    if (!original.empty()) {
        backup("os_release", original);
        
        std::string newContent = "NAME=\"Ubuntu\"\n"
                                 "VERSION=\"20.04.1 LTS (Focal Fossa)\"\n"
                                 "ID=ubuntu\n"
                                 "ID_LIKE=debian\n"
                                 "PRETTY_NAME=\"Ubuntu 20.04.1 LTS\"\n"
                                 "VERSION_ID=\"20.04\"\n"
                                 "HOME_URL=\"https://www.ubuntu.com/\"\n"
                                 "SUPPORT_URL=\"https://help.ubuntu.com/\"\n"
                                 "BUG_REPORT_URL=\"https://bugs.launchpad.net/ubuntu/\"\n"
                                 "PRIVACY_POLICY_URL=\"https://www.ubuntu.com/legal/terms-and-policies/privacy-policy\"\n"
                                 "VERSION_CODENAME=focal\n"
                                 "UBUNTU_CODENAME=focal\n";

        writeToFile(osReleasePath, newContent);
        std::cout << "OS Release spoofed." << std::endl;
    }
}

void Spoofer::restoreOsRelease() {
    restore("os_release", osReleasePath);
}

void Spoofer::spoofSshFingerprint() {
    std::string original = readFromFile(sshConfigPath);
    if(!original.empty()) {
        backup("ssh_config", original);

        std::string newContent = original +
            "\n\n# --- CHIMERA SPOOFED CONFIG ---\n"
            "Ciphers aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com\n"
            "MACs hmac-sha2-256,hmac-sha2-512,hmac-sha1\n"
            "KexAlgorithms curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521\n"
            "HostKeyAlgorithms ssh-ed25519-cert-v01@openssh.com,ssh-rsa-cert-v01@openssh.com,ssh-ed25519,ssh-rsa\n";
        
        writeToFile(sshConfigPath, newContent);
        std::cout << "SSH client fingerprint spoofed." << std::endl;
    }
}

void Spoofer::restoreSshFingerprint() {
    restore("ssh_config", sshConfigPath);
}

void Spoofer::spoofIpIdFingerprint() {
    std::string originalRange = exec("sysctl net.ipv4.ip_local_port_range");
    if(!originalRange.empty()) {
        backup("ip_local_port_range", originalRange);
        std::string command = "sysctl -w net.ipv4.ip_local_port_range=\"32768 60999\"";
        exec(command.c_str());
        std::cout << "Spoofed ip_local_port_range to 32768-60999 (common Linux default)." << std::endl;
    }
}

void Spoofer::restoreIpIdFingerprint() {
    std::string originalRange = readFromBackup("ip_local_port_range");
    if(!originalRange.empty()) {
        // The output of sysctl is "net.ipv4.ip_local_port_range = 32768\t60999"
        // We need to extract the "32768 60999" part.
        std::string range = originalRange.substr(originalRange.find("=") + 2);
        std::replace(range.begin(), range.end(), '\t', ' ');
        std::string command = "sysctl -w net.ipv4.ip_local_port_range=\\\"" + range + "\\\"";
        exec(command.c_str());
        remove((backupDir + "ip_local_port_range").c_str());
        std::cout << "Restored ip_local_port_range." << std::endl;
    }
}

void Spoofer::spoofTimestamps(const std::string& path, const std::string& timestamp) {
    std::string command = "touch -d '" + timestamp + "' " + path;
    exec(command.c_str());
    std::cout << "Set timestamp of " << path << " to " << timestamp << std::endl;
}

void Spoofer::restoreTimestamps(const std::string& path) {
    std::string command = "touch " + path;
    exec(command.c_str());
    std::cout << "Restored timestamp of " << path << " to current time." << std::endl;
}

namespace {
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
}

#endif // __linux__ 