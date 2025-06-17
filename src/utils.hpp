#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

void secureDelete(const std::string& path);
std::string exec(const char* cmd);
std::string generateRandomId(size_t length);

#endif // UTILS_HPP
