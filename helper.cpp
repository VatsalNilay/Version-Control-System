#include "headers.h"
using namespace std;

string sha1_hash(string& data) 
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

string compress_data(string& data) 
{
    uLongf compressed_size = compressBound(data.size());
    vector<char> compressed_data(compressed_size);
    compress(reinterpret_cast<Bytef*>(compressed_data.data()), &compressed_size, reinterpret_cast<const Bytef*>(data.c_str()), data.size());
    return string(compressed_data.begin(), compressed_data.begin() + compressed_size);
}

