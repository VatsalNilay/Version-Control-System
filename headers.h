#include<iostream>
#include<vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <zlib.h>
#include <filesystem>
#include <map>


#define red "\033[31m"
#define green "\033[32m"
#define yellow "\033[33m"
#define blue "\033[34m"
#define magenta "\033[35m"
#define cyan "\033[36m"
#define white "\033[37m"
#define reset "\033[0m"
#define bold "\033[1m"

#define endl "\n"

void handle_init();
void handle_object_hashing(std::vector<std::string>&commands);
void handle_cat_file(std::vector<std::string>&commands);
std::string handle_write_tree(int flag);
void handle_list_tree(std::vector<std::string>&commands);
std::string sha1_hash(std::string& data);
std::string compress_data(std::string& data);
void handle_add_command(std::vector<std::string>&commands);
void handle_commit_command(std::vector<std::string>&commands);
void handle_log_command();
void handle_checkout_command(std::vector<std::string>&commands);
