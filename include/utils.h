#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string Hash(const std::string &content);

std::string getCurrentTime();

extern const std::string Code_Vault;
extern const std::string Objects;
extern const std::string Commits;
extern const std::string Head_File;
extern const std::string Index_File;
extern const std::string Branch_File;

#endif 
