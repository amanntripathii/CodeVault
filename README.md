# 🚀 CodeVault — A Lightweight Version Control System in C++

**CodeVault** is a Git-inspired version control system built from scratch in **C++**.  
It tracks file changes, manages repository states, and supports branching and merging through a command-line interface.

This project focuses on understanding how modern version control systems work internally using hashing, indexing, and structured storage.

---

## ✨ Features

- 📁 **Repository Initialization** – Create a structured version-controlled workspace  
- 📌 **Staging Area** – Track files before committing  
- 📝 **Commit System** – Snapshot-based file versioning using hashing  
- 📜 **Commit Logs** – View project history  
- 🌿 **Branching** – Create and switch between development branches  
- 🔄 **Checkout** – Restore files to previous states  
- ⚔️ **Merge** – Combine changes from multiple branches  
- 🔍 **Commit Difference** – Identify file-level changes between commits  

---

## 🧠 Core Concepts Implemented

This project demonstrates:

- File system manipulation using C++  
- Hash-based content tracking  
- Indexing for staged files  
- Repository state management  
- Branch and merge logic  
- Pointer-based commit linking  
- CLI tool architecture  
- Structured storage schema for versioned data  

---

## 🏗️ System Architecture

CodeVault maintains a hidden repository folder that stores:

- Object database (hashed file contents)  
- Commit metadata  
- Branch references  
- Index/staging information  

Each commit is represented as a structured object linked to previous states, enabling efficient version tracking similar to real VCS systems.

---

## 🛠️ Tech Stack

| Category | Technology |
|---------|-----------|
| Language | C++ |
| Concepts | File I/O, Hashing, Data Structures |
| Interface | Command Line (CLI) |
| Paradigm | Systems Programming |

---

## ▶️ Usage Example

```bash
codevault init
codevault add file.txt
codevault commit "Initial commit"
codevault log
codevault branch feature-x
codevault checkout feature-x
codevault merge main
