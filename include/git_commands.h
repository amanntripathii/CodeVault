#ifndef GIT_COMMANDS_H
#define GIT_COMMANDS_H

#include <string>

void init();
void add(const std::string &filename);
void commit(const std::string &message);
void log();
void createBranch(const std::string &branchName);
void checkout(const std::string &target);
void merge(const std::string &targetBranch);
void diff(const std::string &commit1, const std::string &commit2);

#endif 
