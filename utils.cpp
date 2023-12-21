#include "utils.h"

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
