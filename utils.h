#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem; 
using ch = std::chrono::system_clock;

extern bool file_is_empty(const std::string* filePath);
extern bool search_line(std::ifstream* file_ptr, const std::string& targetLine, std::string* line_ptr);
extern unsigned char* createFileHash(const char* filePath);
extern unsigned char* createHash(const char* input);
extern std::string convertHashToString(unsigned char* hash_ptr, size_t hashSize);
extern std::array<std::string, 2> get_time_array();
extern std::string get_last_line(std::ifstream* file);
extern std::string extract_hash(const std::string& line);

#endif
