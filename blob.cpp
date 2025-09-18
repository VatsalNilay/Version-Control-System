#include "headers.h"
using namespace std;


class Blob {
    string data;
public:
    Blob(string& data) : data(data) {}
    Blob(){}
    string getData() 
    {
        return data;
    }

    string getHash() 
    {
        return sha1_hash(data);
    }

};

class Tree 
{
    map<string, Blob> blobs;
public:
    void addBlob(const string& name, const Blob& blob) 
    {
        blobs[name] = blob;
    }

    string getHash() {
        string combined_data;
        for (auto& [name, blob] : blobs) {
            combined_data += name + blob.getHash();
        }
        return sha1_hash(combined_data);
    }

    void writeTree() 
    {
        for (auto& [name, blob] : blobs) {
            string hash = blob.getHash();
            string path = ".mygit/objects/" + hash.substr(0, 2);
            if (!filesystem::exists(path))
                filesystem::create_directories(path);

            path += "/" + hash.substr(2);
            ofstream file(path, ios::binary);
            string data = blob.getData();
            file.write(data.c_str(), data.size());
        }
    }

};

void handle_object_hashing(vector<string>& commands) 
{
    if (commands.size() < 2) 
    {
        cerr << "Incorrect usage of command." << endl;
        return;
    }

    string file_path = commands.back();
    bool isDir = filesystem::is_directory(file_path);

    if (isDir) 
    {
        Tree tree;
        string dir_name = filesystem::path(file_path).filename().string();
        string contents;
        for (auto& entry : filesystem::recursive_directory_iterator(file_path)) 
        {
            if (!filesystem::is_directory(entry.path())) 
            {
                ifstream subfile(entry.path(), ios::binary);
                stringstream subbuffer;
                subbuffer << subfile.rdbuf();
                string subfile_data = subbuffer.str();
                Blob blob(subfile_data);
                tree.addBlob(entry.path().filename().string(), blob);
                contents += entry.path().filename().string() + ",";
            }
        }
        if (!contents.empty()) {
            contents.pop_back(); // Remove the trailing comma
        }
        string tree_hash = tree.getHash();
        if (commands.size() > 2 && commands[2] == "-w") 
        {
            tree.writeTree();
            string path = ".mygit/objects/" + tree_hash.substr(0, 2);
            if (!filesystem::exists(path))
                filesystem::create_directories(path);

            path += "/" + tree_hash.substr(2);
            ofstream outfile(path, ios::binary);
            string data = "tree\n" + dir_name + "\n" + contents + "\n" + tree_hash;
            outfile.write(data.c_str(), data.size());
        }
        cout << "SHA-1: " << tree_hash << endl;
    } 
    else 
    {
        ifstream file(file_path, ios::binary);
        if (!file) 
        {
            perror("Error: Cannot open file ");
            return;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string file_data = buffer.str();
        Blob blob(file_data);
        string blob_hash = blob.getHash();
        if (commands.size() > 2 && commands[2] == "-w") 
        {
            string path = ".mygit/objects/" + blob_hash.substr(0, 2);
            if (!filesystem::exists(path))
                filesystem::create_directories(path);

            path += "/" + blob_hash.substr(2);
            ofstream outfile(path, ios::binary);
            string data = "blob\n" + blob.getData();
            outfile.write(data.c_str(), data.size());
        }
        cout << "SHA-1: " << blob_hash << endl;
    }
}

void handle_cat_file(vector<string>& commands) 
{
    if (commands.size() < 3) 
    {
        cerr << "Incorrect usage of command." << endl;
        return;
    }

    string flag = commands[2];
    string hash = commands[3];
    string path = ".mygit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);

    if (!filesystem::exists(path)) 
    {
        cerr << "Object not found." << endl;
        return;
    }

    ifstream file(path, ios::binary);
    if (!file) 
    {
        perror("Error: Cannot open file ");
        return;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string file_data = buffer.str();

    if (flag == "-p") 
    {
        cout << file_data << endl;
    } 
    else if (flag == "-s") 
    {
        cout << "Size: " << file_data.size() << " bytes" << endl;
    } 
    else if (flag == "-t") 
    {
        if (file_data.find("tree") != string::npos) 
        {
            cout << "Type: tree" << endl;
        } 
        else 
        {
            cout << "Type: blob" << endl;
        }
    } 
    else 
    {
        cerr << "Unknown flag." << endl;
    }
}
