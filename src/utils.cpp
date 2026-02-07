#include "../include/utils.h"
#include <vector>
#include <ctime>
#include <functional>
using namespace std;

const string Code_Vault = ".mygit";
const string Objects = Code_Vault + "/objects";
const string Commits = Code_Vault + "/commits";
const string Head_File = Code_Vault + "/HEAD.txt";
const string Index_File = Code_Vault + "/index.txt";
const string Branch_File = Code_Vault + "/branches.txt";

std::string Hash(const std::string &content)
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(content));
}

std::string getCurrentTime()
{
    time_t now = time(0);
    char buf[sizeof "YYYY-MM-DD HH:MM:SS"] = {0};
    strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}
