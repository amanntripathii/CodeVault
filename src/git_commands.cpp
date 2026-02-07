#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include "../include/git_commands.h"
#include "../include/utils.h"

namespace fs = std::filesystem;
using namespace std;

string getCurrentBranch();
string getParentCommit(const string &branchName);
void updateLatestCommit(const string &branch, const string &newHash);

void init()
{
    if (fs::exists(Code_Vault))
    {
        cout << "Vault is already initialized." << endl;
        return;
    }
    fs::create_directory(Code_Vault);
    fs::create_directory(Objects);
    fs::create_directory(Commits);
    ofstream(Head_File) << "main";           
    ofstream(Branch_File) << "main:null\n"; 
    ofstream(Index_File);                    

    cout << "Initialized empty Vault repository." << endl;
}

void add(const string &filename)
{
    ifstream inFile(filename);
    if (!inFile)
    {
        cout << "File not found: " << filename << endl;
        return;
    }
    stringstream buffer;
    buffer << inFile.rdbuf();
    string content = buffer.str();
    string hash = Hash(content);

    ofstream outFile(Objects + "/" + hash);
    outFile << content;
    outFile.close();

    ofstream indexOut(Index_File, ios::app);
    indexOut << filename << ":" << hash << endl;

    cout << "Staged file: " << filename << endl;
}

void commit(const string &message)
{
    ifstream indexIn(Index_File);
    if (indexIn.peek() == ifstream::traits_type::eof())
    {
        cout << "There is nothing to commit." << endl;
        return;
    }

    stringstream commitData;
    string line;
    while (getline(indexIn, line))
    {
        commitData << line << endl;
    }

    string timestamp = getCurrentTime();
    string branch = getCurrentBranch();
    string parent = getParentCommit(branch);

    string metadata = "message: " + message + "\n" +
                      "timestamp: " + timestamp +
                      "parent: " + parent + "\n" +
                      "branch: " + branch + "\n";

    string commitHash = Hash(metadata + commitData.str());
    ofstream commitOut(Commits + "/" + commitHash);
    commitOut << metadata;
    commitOut << commitData.str();

    updateLatestCommit(branch, commitHash);

    ofstream(Index_File, ofstream::out | ofstream::trunc); 
    cout << "Committed. Hash: " << commitHash << endl;
}

void log()
{
    string branch = getCurrentBranch();
    string current = getParentCommit(branch);

    while (current != "null")
    {
        string path = Commits + "/" + current;
        ifstream in(path);
        string line;
        cout << "Commit: " << current << endl;
        while (getline(in, line))
        {
            if (line.rfind("message:", 0) == 0 ||
                line.rfind("timestamp:", 0) == 0 ||
                line.rfind("parent:", 0) == 0)
            {
                cout << line << endl;
            }
        }
        cout << "-------------------------------------------------------------" << endl;

        in.clear();
        in.seekg(0, ios::beg);
        bool found = false;
        while (getline(in, line))
        {
            if (line.rfind("parent: ", 0) == 0)
            {
                current = line.substr(8);
                found = true;
                break;
            }
        }
        if (!found)
            current = "null";
    }
}

void createBranch(const string &branchName)
{
    string current = getParentCommit(getCurrentBranch());
    ofstream branchesOut(Branch_File, ios::app);
    branchesOut << branchName << ":" << current << "\n";
    cout << "Created branch: " << branchName << endl;
}

void checkout(const string &target)
{
    string hash = getParentCommit(target);
    if (hash != "null")
    {
        ofstream(Head_File) << target;
        cout << "Switched to branch: " << target << endl;
        return;
    }

    if (fs::exists(Commits + "/" + target))
    {
       
        cout << "Checked out commit (read-only): " << target << endl;
    }
    else
    {
        cout << "Branch or commit not found: " << target << endl;
    }
}

void merge(const string &targetBranch)
{
    string currentBranch = getCurrentBranch();
    string currentCommit = getParentCommit(currentBranch);
    string targetCommit = getParentCommit(targetBranch);

    if (targetCommit == "null")
    {
        cout << "Branch not found or no commits in target branch." << endl;
        return;
    }

    ifstream targetIn(Commits + "/" + targetCommit);
    string line;
    unordered_map<string, string> mergedFiles;

    while (getline(targetIn, line))
    {
        if (line.find(":") != string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0)
        {
            size_t sep = line.find(":");
            string file = line.substr(0, sep);
            string hashVal = line.substr(sep + 1);
            mergedFiles[file] = hashVal;
        }
    }

    ifstream currentIn(Commits + "/" + currentCommit);
    while (getline(currentIn, line))
    {
        if (line.find(":") != string::npos && line.find("message:") != 0 &&
            line.find("timestamp:") != 0 && line.find("parent:") != 0 &&
            line.find("branch:") != 0)
        {
            size_t sep = line.find(":");
            string file = line.substr(0, sep);
            string hashVal = line.substr(sep + 1);
            if (mergedFiles.count(file) && mergedFiles[file] != hashVal)
            {
                cout << "CONFLICT: both modified " << file << endl;
            }
            mergedFiles[file] = hashVal; 
        }
    }

    ofstream indexOut(Index_File);
    for (const auto &[file, hashVal] : mergedFiles)
    {
        indexOut << file << ":" << hashVal << endl;
    }

    stringstream msg;
    msg << "Merged branch " << targetBranch;
    commit(msg.str());
}

void diff(const string &commit1, const string &commit2)
{
    ifstream in1(Commits + "/" + commit1);
    ifstream in2(Commits + "/" + commit2);
    if (!in1 || !in2)
    {
        cout << "One of the commits or both are not found." << endl;
        return;
    }

    unordered_map<string, string> map1, map2;
    string line;
    auto readFileHashes = [&](ifstream &in, unordered_map<string, string> &map)
    {
        while (getline(in, line))
        {
            if (line.find(":") != string::npos && line.find("message:") != 0 && line.find("timestamp:") != 0 && line.find("parent:") != 0 && line.find("branch:") != 0)
            {
                size_t sep = line.find(":");
                map[line.substr(0, sep)] = line.substr(sep + 1);
            }
        }
    };

    readFileHashes(in1, map1);
    readFileHashes(in2, map2);

    cout << "### Diff between " << commit1 << " and " << commit2 << " ###" << endl;
    for (const auto &[file, hash1] : map1)
    {
        if (map2.find(file) == map2.end())
        {
            cout << "- " << file << " (removed in " << commit2 << ")" << endl;
        }
        else if (map2[file] != hash1)
        {
            cout << "M " << file << " (modified)" << endl;
        }
    }
    for (const auto &[file, hash2] : map2)
    {
        if (map1.find(file) == map1.end())
        {
            cout << "+ " << file << " (added in " << commit2 << ")" << endl;
        }
    }
}

string getCurrentBranch()
{
    ifstream headIn(Head_File);
    string branch;
    getline(headIn, branch);
    return branch;
}

string getParentCommit(const string &branchName)
{
    ifstream branchesIn(Branch_File);
    string line;
    while (getline(branchesIn, line))
    {
        size_t sep = line.find(":");
        if (line.substr(0, sep) == branchName)
        {
            return line.substr(sep + 1);
        }
    }
    return "null";
}

void updateLatestCommit(const string &branch, const string &newHash)
{
    ifstream in(Branch_File);
    stringstream updated;
    string line;
    bool branchFound = false;
    while (getline(in, line))
    {
        size_t sep = line.find(":");
        string name = line.substr(0, sep);
        if (name == branch)
        {
            updated << name << ":" << newHash << "\n";
            branchFound = true;
        }
        else
        {
            updated << line << "\n";
        }
    }
    in.close();

    if (!branchFound)
    {
        updated << branch << ":" << newHash << "\n";
    }

    ofstream out(Branch_File);
    out << updated.str();
}
