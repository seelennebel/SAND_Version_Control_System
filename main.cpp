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

namespace fs = std::filesystem;
using ch = std::chrono::system_clock;

bool file_is_empty(const std::string* filePath)
        {
            if(fs::exists(*filePath))
            {
                return fs::file_size(*filePath) == 0;
            }
            else
            {
                return false;
            }
        }

bool search_line(std::ifstream* file_ptr, const std::string& targetLine, std::string* line_ptr)
        {
            if(file_ptr->is_open() && file_ptr)
            {
                file_ptr->clear();
                file_ptr->seekg(0, file_ptr->beg);
                *line_ptr = "";
                while(std::getline(*file_ptr, *line_ptr))
                {
                    if(*line_ptr == targetLine)
                    {
                        return true;
                        break;
                    }
                }
            }
            return false;
        }

unsigned char* createFileHash(const char* filePath)
{
    std::ifstream file(filePath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    const unsigned char* data = reinterpret_cast<const unsigned char*>(content.c_str());
    size_t count = content.size();
    unsigned char* buffer = new unsigned char[SHA256_DIGEST_LENGTH];

    SHA256(data, count, buffer);

    return buffer;
}

unsigned char* createHash(const char* input)
{
    const unsigned char* data = reinterpret_cast<const unsigned char*>(input);
    size_t count = strlen(input);
    unsigned char* buffer = new unsigned char[SHA256_DIGEST_LENGTH];

    SHA256(data, count, buffer);

    return buffer;
}

std::string convertHashToString(unsigned char* hash_ptr, size_t hashSize = SHA256_DIGEST_LENGTH)
{
    std::stringstream ss;
    for(size_t i = 0; i < hashSize; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash_ptr[i]);
    }
    delete[] hash_ptr;
    return ss.str();
}

std::array<std::string, 2> get_time_array()
{
   auto now = ch::now();
   auto epoh = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
   std::time_t currentTime = ch::to_time_t(now);
   std::string currentTimeStamp = std::ctime(&currentTime);
   std::stringstream ss;
   ss << epoh;
   return {currentTimeStamp, ss.str()};
}

std::string get_last_line(std::ifstream* file)
{
    std::string line;
    std::string last;
    file->seekg(0, std::ios::beg);
    while(std::getline(*file, line))
    {
        last = line;
    }
    return last;
}

std::string extract_hash(const std::string& line)
        {
            std::stringstream ss;
            size_t hashStart = line.find('|') + 2;
            size_t hashEnd = line.find('|', hashStart);

            if(hashStart != std::string::npos && hashEnd != std::string::npos)
            {
                return line.substr(hashStart, hashEnd - hashStart - 1);
            }
            return "";
        }

class VersionControlSystem
{
    private:

        fs::path mainFilePath = fs::current_path();
        const char* sandUsageString = "usage: sand [init] [track <filename>] \n";
        const char* sandFileName = "sand_tracked_files.txt";
        const char* sandDirectoryName = ".sand";
        const char* commitDirectoryName = "commits";
        const char* commitFileName = "commit_logs.txt";

        fs::path commitDirectoryPath = mainFilePath / sandDirectoryName / commitDirectoryName;
        std::string commitFilePath = mainFilePath / sandDirectoryName / commitFileName;
        std::string sandFilePath = mainFilePath / sandDirectoryName / sandFileName;
        std::string sandDirectoryPath = mainFilePath / sandDirectoryName;

    public:

        friend unsigned char* createFileHash(const char* filePath);
        friend bool search_line(std::ifstream* file_ptr, const std::string& targetLine, std::string* line_ptr);
        friend std::string convertHashToString(unsigned char* hash_ptr, size_t hashSize);
        friend unsigned char* createHash(const char* input);
        friend std::array<std::string, 2> get_time_array();
        friend std::string extract_hash(const std::string& line);
        friend std::string get_last_line(std::ifstream* file);

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
                    std::string commitHash = convertHashToString(createFileHash((commitDirectoryPath / lastLineHash / str).c_str()));
                    std::string trackHash = convertHashToString(createFileHash((str).c_str()));

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
            sandInputFile.close();
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
        if(strcmp(*(argv+1), "track") == 0)
        {
            VCS.track(argc, argv);
        }
        if(strcmp(*(argv+1), "commit") == 0)
        {
            VCS.commit();
        }
        if(strcmp(*(argv+1), "revert") == 0)
        {
            VCS.revert(argv[2]);
        }
    }
    return 0;
}