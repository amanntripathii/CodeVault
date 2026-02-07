#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <ctime>
#include <sstream>

std::string Hash(const std::string &content)
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(content));
}

const std::string Code_Vault = ".vault";
const std::string Objects = Code_Vault + "/objects";
const std::string Commits = Code_Vault + "/commits";
const std::string Head_File = Code_Vault + "/HEAD.txt";
const std::string Index_File = Code_Vault + "/index.txt";
const std::string Branch_File = Code_Vault + "/branches.txt";

std::string getCurrentTime()
{
    std::time_t now = std::time(0);
    char *dt = std::ctime(&now);
    return std::string(dt);
}

void init()
{
    if (std::filesystem::exists(Code_Vault))
    {
        std::cout << "vault is already initialized." << std::endl;
        return;
    }
    std::filesystem::create_directory(Code_Vault);
    std::filesystem::create_directory(Objects);
    std::filesystem::create_directory(Commits);
    std::ofstream(Head_File) << "main";
    std::ofstream(Branch_File) << "main:null\n";
    std::ofstream(Index_File);
    std::cout << "Initialized empty vault repository." << std::endl;
}

void ensureBranchExists(const std::string &branch)
{
    std::ifstream in(Branch_File);
    std::string line;
    while (std::getline(in, line))
    {
        size_t sep = line.find(":");
        if (sep != std::string::npos && line.substr(0, sep) == branch)
            return;
    }
    std::ofstream out(Branch_File, std::ios::app);
    out << branch << ":null\n";
}

void add(const std::string &filename)
{
    std::ifstream inFile(filename);
    if (!inFile)
    {
        std::cout << "File not found: " << filename << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();
    std::string hash = Hash(content);
    std::ofstream outFile(Objects + "/" + hash);
    outFile << content;
    outFile.close();
    std::ofstream indexOut(Index_File, std::ios::app);
    indexOut << filename << ":" << hash << std::endl;
    std::cout << "Staged file: " << filename << std::endl;
}

std::string getCurrentBranch()
{
    std::ifstream headIn(Head_File);
    std::string branch;
    std::getline(headIn, branch);
    return branch;
}

std::string getParentCommit(const std::string &branchName)
{
    std::ifstream branchesIn(Branch_File);
    std::string line;
    while (std::getline(branchesIn, line))
    {
        size_t sep = line.find(":");
        if (line.substr(0, sep) == branchName)
            return line.substr(sep + 1);
    }
    return "null";
}

void updateLatestCommit(const std::string &branch, const std::string &newHash)
{
    std::ifstream in(Branch_File);
    std::stringstream updated;
    std::string line;
    bool found = false;
    while (std::getline(in, line))
    {
        size_t sep = line.find(":");
        std::string name = line.substr(0, sep);
        if (name == branch)
        {
            updated << name << ":" << newHash << "\n";
            found = true;
        }
        else
            updated << line << "\n";
    }
    in.close();
    if (!found)
        updated << branch << ":" << newHash << "\n";
    std::ofstream out(Branch_File);
    out << updated.str();
}

void commit(const std::string &message)
{
    std::ifstream indexIn(Index_File);
    if (!indexIn)
    {
        std::cout << "No files staged for commit.\n";
        return;
    }

    std::stringstream commitData;
    std::string line;
    bool hasFiles = false;

    while (std::getline(indexIn, line))
    {
        if (!line.empty())
        {
            commitData << line << "\n";
            hasFiles = true;
        }
    }
    indexIn.close();

    if (!hasFiles)
    {
        std::cout << "Nothing to commit.\n";
        return;
    }

    std::string branch = getCurrentBranch();
    ensureBranchExists(branch);
    std::string timestamp = getCurrentTime();
    std::string parent = getParentCommit(branch);

    std::string metadataWithoutHash =
        "message: " + message + "\n" +
        "timestamp: " + timestamp +
        "parent: " + parent + "\n" +
        "branch: " + branch + "\n";

    std::string tempHash = Hash(metadataWithoutHash + commitData.str());

    std::string metadata =
        "message: " + message + "\n" +
        "timestamp: " + timestamp +
        "parent: " + parent + "\n" +
        "branch: " + branch + "\n" +
        "current_hash: " + tempHash + "\n";

    std::string commitPath = Commits + "/" + tempHash;
    if (!std::filesystem::exists(commitPath))
    {
        std::ofstream commitOut(commitPath);
        commitOut << metadata << commitData.str();
        commitOut.close();
    }

    updateLatestCommit(branch, tempHash);

    std::ofstream clear(Index_File, std::ios::trunc);
    clear.flush();
    clear.close();

    std::cout << "Committed successfully. Hash: " << tempHash << "\n";
}

void log()
{
    std::string branch = getCurrentBranch();
    std::string current = getParentCommit(branch);
    while (current != "null")
    {
        std::string path = Commits + "/" + current;
        std::ifstream in(path);
        std::string line;
        std::cout << "Commit: " << current << std::endl;
        while (std::getline(in, line))
        {
            if (line.rfind("message:", 0) == 0 ||
                line.rfind("timestamp:", 0) == 0 ||
                line.rfind("parent:", 0) == 0 ||
                line.rfind("current_hash:", 0) == 0)
                std::cout << line << std::endl;
        }
        std::cout << "-------------------------------------------------------------" << std::endl;
        in.clear();
        in.seekg(0, std::ios::beg);
        while (std::getline(in, line))
        {
            if (line.rfind("parent:", 0) == 0)
            {
                current = line.substr(8);
                break;
            }
        }
    }
}

void createBranch(const std::string &branchName)
{
    std::string current = getParentCommit(getCurrentBranch());
    std::ofstream branchesOut(Branch_File, std::ios::app);
    branchesOut << branchName << ":" << current << "\n";
    std::cout << "Created branch: " << branchName << std::endl;
}

void checkout(const std::string &target)
{
    std::string hash = getParentCommit(target);
    if (hash != "null")
    {
        std::ofstream(Head_File) << target;
        std::cout << "Switched to branch: " << target << std::endl;
        return;
    }
    if (std::filesystem::exists(Commits + "/" + target))
        std::cout << "Checked out commit: " << target << std::endl;
    else
        std::cout << "Branch or commit not found: " << target << std::endl;
}

void merge(const std::string &targetBranch)
{
    std::string currentBranch = getCurrentBranch();
    std::string currentCommit = getParentCommit(currentBranch);
    std::string targetCommit = getParentCommit(targetBranch);
    if (targetCommit == "null")
    {
        std::cout << "Branch not found or no commits in target branch." << std::endl;
        return;
    }

    std::ifstream targetIn(Commits + "/" + targetCommit);
    std::string line;
    std::unordered_map<std::string, std::string> mergedFiles;

    while (std::getline(targetIn, line))
    {
        if (line.find(":") != std::string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0 && line.find("current_hash:") != 0)
        {
            size_t sep = line.find(":");
            std::string file = line.substr(0, sep);
            std::string hash = line.substr(sep + 1);
            mergedFiles[file] = hash;
        }
    }

    std::ifstream currentIn(Commits + "/" + currentCommit);
    while (std::getline(currentIn, line))
    {
        if (line.find(":") != std::string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0 && line.find("current_hash:") != 0)
        {
            size_t sep = line.find(":");
            std::string file = line.substr(0, sep);
            std::string hash = line.substr(sep + 1);
            if (mergedFiles.find(file) != mergedFiles.end() && mergedFiles[file] != hash)
                std::cout << "CONFLICT: both modified " << file << std::endl;
            mergedFiles[file] = hash;
        }
    }

    std::ofstream indexOut(Index_File);
    for (const auto &[file, hash] : mergedFiles)
        indexOut << file << ":" << hash << std::endl;

    std::stringstream msg;
    msg << "Merged branch " << targetBranch;
    commit(msg.str());
}

void diff(const std::string &commit1, const std::string &commit2)
{
    std::ifstream in1(Commits + "/" + commit1);
    std::ifstream in2(Commits + "/" + commit2);
    if (!in1 || !in2)
    {
        std::cout << "One of the commits or both are not found." << std::endl;
        return;
    }

    std::unordered_map<std::string, std::string> map1, map2;
    std::string line;
    while (std::getline(in1, line))
    {
        if (line.find(":") != std::string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0 && line.find("current_hash:") != 0)
        {
            size_t sep = line.find(":");
            map1[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }
    while (std::getline(in2, line))
    {
        if (line.find(":") != std::string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0 && line.find("current_hash:") != 0)
        {
            size_t sep = line.find(":");
            map2[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }

    std::cout << "### Diff between " << commit1 << " and " << commit2 << " ###" << std::endl;
    for (const auto &[file, hash1] : map1)
    {
        if (map2.find(file) == map2.end())
            std::cout << file << " was removed in " << commit2 << std::endl;
        else if (map2[file] != hash1)
            std::cout << file << " was modified." << std::endl;
    }
    for (const auto &[file, hash2] : map2)
    {
        if (map1.find(file) == map1.end())
            std::cout << file << " was added in " << commit2 << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "If vault is not added to our system add that using following commands: " << std::endl
                  << std::endl;
        std::cout << "1. ./vault init" << std::endl;
        std::cout << "2. ./vault add <file>" << std::endl;
        std::cout << "3. ./vault commit -m 'commit message goes here'" << std::endl;
        std::cout << "4. ./vault log" << std::endl;
        std::cout << "5. ./vault branch branch_name" << std::endl;
        std::cout << "6. ./vault checkout branch_name" << std::endl;
        std::cout << "7. ./vault merge branch_name" << std::endl;
        std::cout << "8. ./vault diff <commit1> <commit2>" << std::endl
                  << std::endl;
        return 1;
    }

    std::string command = argv[1];
    if (command == "init")
        init();
    else if (command == "add" && argc >= 3)
        add(argv[2]);
    else if (command == "commit" && argc >= 4 && std::string(argv[2]) == "-m")
        commit(argv[3]);
    else if (command == "log")
        log();
    else if (command == "branch" && argc >= 3)
        createBranch(argv[2]);
    else if (command == "checkout" && argc >= 3)
        checkout(argv[2]);
    else if (command == "merge" && argc >= 3)
        merge(argv[2]);
    else if (command == "diff" && argc >= 4)
        diff(argv[2], argv[3]);
    else
        std::cout << "Unknown or incomplete command." << std::endl;

    return 0;
}
