# SAND_Version_Control_System
The SAND version control system allows tracking files, committing changes, and reverting to previous commits

## Overview
The code was initially compiled using: 
- Apple Clang version 14.0.3 (default Apple macOS compiler)
- C++17 Standart Library
- OpenSSL library

## Installation (for macOS)
1. Check if you have a c++ compiler installed (clang++, g++)
2. Check if you have the OpenSSL library installed

   ```
   git clone https://github.com/seelennebel/SAND_Version_Control_System
   cd SAND_Version_Control_System
   SAND_Version_Control_System % clang++ -std=c++17 -o sand -I/path/to/openssl/include -L/path/to/openssl/lib -lcrypto -lssl main.cpp utils.cpp

The output of the compiler will be a "sand" executable. You can move executable to the directory that you want to track. On WINDOWS systems the installations steps can differ depending on compiler and shell

## Features
- **Help**: display a message that explains the usage of all commands. Execute the program (*./sand*) without any parameters to see commands or add *help* flag to see explanations of commands

  **Example**:
  ```
  ./sand
  usage: sand [help] [init] [track <filename>] [commit] [revert <commit hash>] [log] [integrity]
  ./sand help
  [help] -- get information about commands
  [init] -- initialize .sand directory for commits
  [track <filename>] -- track file with the name <filename>
  [commit] -- save changes of tracked files
  [revert <commit hash>] -- revert the tracked files to the commit with <commit hash>
  [log] -- view the history of commits
  [integrity] -- check if commits are fine
  
- **Initialization**: create a directory where SAND will store its files

  **Example**:
  ```
  ./sand init
  tree .sand
  .sand
  ├── commit_logs.txt
  ├── commits
  └── sand_tracked_files.txt

- **Track**: track files in the current directory. The command will add the name of a file to the *sand_tracked_files.txt*. You can also change tracked files manually by editing *sand_tracked_files.txt*. To see all tracked files use *./sand track*

  **Example**:
  ```
  touch test.txt
  ./sand track test.txt
  File test.txt tracked
  cat .sand/sand_tracked_files.txt
  test.txt
  ./sand track
  Tracked files:
  test.txt

- **Commit**: commit changes. SAND will commit the files in their current state, i.e., if the file is not saved, SAND stores unsaved copy of the file. SAND copies file contents to the *commits* folder. The name for each commit is created with a unique hash

  **Example**:
  ```
  ./sand track
  Tracked files:
  test.txt
  seelennebel@MBP test % ./sand commit
  Commit successful
  seelennebel@MBP test % tree .sand
  .sand
  ├── commit_logs.txt
  ├── commits
  │   └── 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56
  │       └── test.txt
  └── sand_tracked_files.txt

- **Revert**: revert to the previous commit by providing the name of the commit. To see the history use *./sand log* command. In the example the *test.txt* file was empty in the first commit

  **Example**:
  ```
  cat test.txt
  testtesttesttesttesttesttesttesttesttesttesttesttesttest%
  ./sand log
  1 | 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56 | Wed Dec 27 15:58:00 2023
  ./sand revert 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56
  Reverted to: 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56
  cat test.txt
  %

- **Integrity**: SAND checks if all commits hold their places

  **Example**:
  ```
  ./sand log
  1 | 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56 | Wed Dec 27 15:58:00 2023
  2 | d8d0b2ef2139380e154cf4eda77f10254273c24e8128361409c335dd56472c95 | Wed Dec 27 16:11:28 2023
  tree .sand
  .sand
  ├── commit_logs.txt
  ├── commits
  │   ├── 7206317bc8253d3de0ec63c0c578b0c44a48f2278965dd7467c751f49a0f1d56
  │   │   └── test.txt
  │   └── d8d0b2ef2139380e154cf4eda77f10254273c24e8128361409c335dd56472c95
  │       └── test.txt
  └── sand_tracked_files.txt
  
  4 directories, 4 files
  rm -r .sand/commits/d8d0b2ef2139380e154cf4eda77f10254273c24e8128361409c335dd56472c95
  ./sand integrity
  WARNING: INTEGRITY CHECK FAILED
  Check .sand/commits directory
 
