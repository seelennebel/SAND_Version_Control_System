#include <iostream>
#include <filesystem>
#include <cstring>
#include <fstream>
#include <string>
#include <openssl/sha.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "utils.h"

namespace fs = std::filesystem;
using ch = std::chrono::system_clock;

class VersionControlSystem
{
    private:

        fs::path mainFilePath = fs::current_path();
        const char* sandUsageString = "usage: sand [help] [init] [track <filename>] [commit] [revert <commit hash>] [log] [integrity]\n";
        const char* sandFileName = "sand_tracked_files.txt";
        const char* sandDirectoryName = ".sand";
        const char* commitDirectoryName = "commits";
        const char* commitFileName = "commit_logs.txt";

        fs::path commitDirectoryPath = mainFilePath / sandDirectoryName / commitDirectoryName;
        std::string commitFilePath = mainFilePath / sandDirectoryName / commitFileName;
        std::string sandFilePath = mainFilePath / sandDirectoryName / sandFileName;
        std::string sandDirectoryPath = mainFilePath / sandDirectoryName;

    public:

        void init()
        {
            fs::create_directory(sandDirectoryPath);
            fs::create_directory(commitDirectoryPath);
            std::fstream sandFile(sandFilePath, std::ios::out);
            std::fstream commitFile(commitFilePath, std::ios::out);

            sandFile.close();
            commitFile.close();
        }

        void usage()
        {
            std::cout << sandUsageString;
        }

        void track(int argc, char* argv[])
        {
            std::ifstream inputFile(sandFilePath);
            std::ifstream* file_ptr = &inputFile;
            std::string line;
            std::string* line_ptr = &line;
            if(inputFile.is_open())
            {
                if (argc == 2)
                {
                    if(file_is_empty(&sandFilePath))
                    {
                        std::cout << "No tracked files \n"; 
                    }
                    else
                    {
                        inputFile.seekg(0, inputFile.beg);
                        
                        std::cout << "Tracked files: " << std::endl;
                        while(std::getline(inputFile, line))
                            std::cout << line << std::endl;
                    }
                }
                else if(argc == 3)
                {
                    if(fs::exists(argv[2]))
                    {
                        if(search_line(file_ptr, argv[2], line_ptr))
                        {
                            std::cout << "The file has been already tracked \n";
                        }
                        else
                        {
                            std::ofstream outputFile(sandFilePath, std::ios::app);
                            outputFile << argv[2] << "\n";
                            std::cout << "File " << argv[2] << " tracked" << "\n";
                            outputFile.close();
                        }
                    }
                    else
                    {
                        std::cout << "File was not found \n";
                    }
                }
            }
            else
            {
                std::cout << "Error with opening tracking file \n";
            }
            inputFile.close();
        }

        bool detect_content_changes()
        {
            try{
                std::ifstream sandInputFile(sandFilePath);
                std::ifstream commitInputFile(commitFilePath);
                std::string line;
                std::vector<std::string> sandFiles;
                std::vector<std::string> commitFiles;
                std::vector<std::string> sortedCommitFiles;
                sandInputFile.seekg(0, std::ios::beg);
                while(std::getline(sandInputFile, line))
                {
                    sandFiles.push_back(line); 
                }
                std::string lastLineHash = extract_hash(get_last_line(&commitInputFile));

                for(const auto& entry : fs::directory_iterator(commitDirectoryPath / lastLineHash))
                {
                    commitFiles.push_back(entry.path().filename());
                }

                if(sandFiles.size() != commitFiles.size())
                {
                    return true;
                }
                else
                {
                    for(std::string str : sandFiles)
                    {
                        std::string commitHash = convertHashToString(createFileHash((commitDirectoryPath / lastLineHash / str).c_str()),SHA256_DIGEST_LENGTH);
                        std::string trackHash = convertHashToString(createFileHash((str).c_str()), SHA256_DIGEST_LENGTH);

                        if(commitHash == trackHash)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    }
                    return false;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
            return false;
        }
        
        void commit() 
        {
            if(!file_is_empty(&sandFilePath))
            {
                if(detect_content_changes() == true)
                {
                    auto time_array = get_time_array();
                    std::ofstream commitOutputFile(commitFilePath, std::ios::app);
                    std::ifstream commitInputFile(commitFilePath);
                    std::stringstream ss;
                    ss << time_array[0] << time_array[1];
                    unsigned char* hashResult = createHash(ss.str().c_str());
                    std::string hashString = convertHashToString(hashResult, SHA256_DIGEST_LENGTH);
                    fs::path newCommitPath = commitDirectoryPath;
                    newCommitPath /= hashString;
                    fs::create_directory(newCommitPath);
                    std::ifstream sandFile(sandFilePath);
                    std::string line;

                    if(file_is_empty(&commitFilePath))
                    {
                        if(sandFile.is_open() && commitOutputFile.is_open() && commitInputFile.is_open())
                        {
                            ss.str("");
                            ss << "1" << " | " << hashString << " | " << time_array[0];
                            commitOutputFile << ss.str();
                            sandFile.seekg(0, sandFile.beg);
                            while(std::getline(sandFile, line))
                            {
                                fs::copy(line, newCommitPath);
                            }
                            commitInputFile.close();
                            commitOutputFile.close();
                            sandFile.close();
                            std::cout << "Commit successful" << "\n";
                            return;
                        }
                    }
                    else
                    {
                        if(sandFile.is_open() && commitOutputFile.is_open() && commitInputFile.is_open())
                        {
                            int place = 1;
                            commitInputFile.seekg(0, std::ios::beg);
                            while(std::getline(commitInputFile, line))
                            {
                                ++place;
                            }
                            sandFile.seekg(0, std::ios::beg);
                            while(std::getline(sandFile, line))
                            {
                                fs::copy(line, newCommitPath);
                            }
                            ss.str("");
                            ss << place << " | " << hashString << " | " << time_array[0];
                            commitOutputFile << ss.str();

                            sandFile.close();
                            commitInputFile.close();
                            commitOutputFile.close();
                            std::cout << "Commit successful" << "\n";
                            return;
                        }
                        else
                        {
                            std::cout << "Error";
                        }
                    }
                }
                else if(detect_content_changes() == false)
                {
                    std::cout << "No detected changes" << "\n";
                    return;
                }
            }
            else
            {
                std::cout << "Nothing to commit" << "\n";
                return;
            }
        }

        void revert(const char* hash)
        {
            std::ifstream sandInputFile(sandFilePath);
            std::vector<std::string> sandFiles;
            std::vector<std::string> commitFiles;
            std::string line;
            if(sandInputFile.is_open())
            {
                sandInputFile.seekg(0, std::ios::beg);
                while(std::getline(sandInputFile, line))
                {
                    sandFiles.push_back(line);
                }

                for(const auto& entry : fs::directory_iterator(commitDirectoryPath / hash))
                {
                    commitFiles.push_back(entry.path().filename());
                }
                if(sandFiles.size() == commitFiles.size())
                {
                    for(const auto& entry : fs::directory_iterator(commitDirectoryPath / hash))
                    {
                        fs::copy_file(entry.path(), mainFilePath / entry.path().filename(), fs::copy_options::overwrite_existing);
                    }
                }
                else
                {
                    for(std::string element : sandFiles)
                    {
                        auto it = std::find(commitFiles.begin(), commitFiles.end(), element);
                        if(it != commitFiles.end())
                        {
                            continue;
                        }
                        else
                        {
                            fs::remove(element);
                        }
                    }
                }
                std::cout << "Reverted to: " << hash << "\n";
                sandInputFile.close();
            }
            else
            {
                std::cerr << "Error" << "\n";
            }
        }

        void view_logs()
        {
            std::ifstream file("./.sand/commit_logs.txt");
            if(file.is_open())
            {
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    std::cout << content;
            }
            else
            {
                std::cerr << "Error with opening logs file" << "\n";
            }
        }

        void help()
        {
            std::cout << "[help] -- get information about commands" << "\n";
            std::cout << "[init] -- initialize .sand directory for commits" << "\n";
            std::cout << "[track <filename>] -- track file with the name <filename>" << "\n";
            std::cout << "[commit] -- save changes of tracked files" << "\n";
            std::cout << "[revert <commit hash>] -- revert the tracked files to the commit with <commit hash>" << "\n";
            std::cout << "[log] -- view the history of commits" << "\n";
            std::cout << "[integrity] -- check if commits are fine" << "\n";
        }

        void integrity()
        {
            std::ifstream file(commitFilePath);
            std::string line;
            if(file.is_open())
            {
                while(std::getline(file, line))
                {
                    if(fs::exists(commitDirectoryPath / extract_hash(line)) && fs::is_directory(commitDirectoryPath / extract_hash(line)))
                    {
                        continue;
                    }
                    else
                    {
                        std::cerr << "WARNING: INTEGRITY CHECK FAILED" << "\n";
                        std::cerr << "Check .sand/commits directory" << "\n";
                        file.close();
                        return;
                    }
                }
            }
            std::cout << "Commits are checked | ALL FINE" << "\n";
            file.close();
            return;
        }
};

int main(int argc, char* argv[])
{
    VersionControlSystem VCS;

    if(argc == 1)
    {
        VCS.usage();
    }
    if(argc > 1)
    {
        if(strcmp(*(argv+1), "init") == 0)
        {
            VCS.init();
        }
        else if(strcmp(*(argv+1), "track") == 0)
        {
            VCS.track(argc, argv);
        }
        else if(strcmp(*(argv+1), "commit") == 0)
        {
            VCS.commit();
        }
        else if(strcmp(*(argv+1), "revert") == 0)
        {
            VCS.revert(argv[2]);
        }
        else if(strcmp(*(argv+1), "log") == 0)
        {
            VCS.view_logs();
        }
        else if(strcmp(*(argv+1), "help") == 0)
        {
            VCS.help();
        }
        else if(strcmp(*(argv+1), "integrity") == 0)
        {
            VCS.integrity();
        }
        else
        {
            std::cerr << "Invalid command" << "\n";
        }
    }
    return 0;
}