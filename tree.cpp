#include "headers.h"
using namespace std;



class Blob {
    string data;
public:
    Blob(const string& data) : data(data) {}
    Blob(){}
    string getData() const { return data; }
    string getHash()
    {
        return sha1_hash(data);
    }

};


class Tree {

public:
    map<string, Blob> blobs;
    map<string, Tree> subtrees;
    void addBlob(const string& name, const Blob& blob) {
        blobs[name] = blob;
    }

    void addSubtree(const string& name, const Tree& tree) {
        subtrees[name] = tree;
    }

    string getHash()  {
        string combined_data;
        for (auto& [name, blob] : blobs) {
            combined_data += "blob " + blob.getHash() + " " + name + "\n";
        }
        for (auto& [name, subtree] : subtrees) {
            combined_data += "tree " + subtree.getHash() + " " + name + "\n";
        }
        return sha1_hash(combined_data);
    }
    void writeTree(string hash)  
    {
        string path = ".mygit/objects/" + hash.substr(0, 2);
        if (!filesystem::exists(path)) {
            filesystem::create_directories(path);
        }

        path += "/" + hash.substr(2);

        ofstream file(path, ios::binary);
        if (file) {
            string combined_data;
            for (auto& [name, blob] : blobs) {
                // Write blobs to disk first
                string blobHash = blob.getHash();
                string blobDir = ".mygit/objects/" + blobHash.substr(0, 2);
                string blobPath = blobDir + "/" + blobHash.substr(2);
                
                if (!filesystem::exists(blobDir)) {
                    filesystem::create_directories(blobDir);
                }
                
                if (!filesystem::exists(blobPath)) {
                    ofstream blobFile(blobPath, ios::binary);
                    if (blobFile) {
                        blobFile << blob.getData();
                        blobFile.close();
                    }
                }
                
                combined_data += "blob " + blobHash + " " + name + "\n";
            }
            for (auto& [name, subtree] : subtrees) {
                string subtreeHash = subtree.getHash();
                subtree.writeTree(subtreeHash);
                combined_data += "tree " + subtreeHash + " " + name + "\n";
            }
            file << combined_data;
            file.close();
        } else {
            cerr << "Error: Could not write to file " << path << endl;
        }
    }
  
};

Tree buildTree(const filesystem::path& directory_path) {
    Tree tree;

    for ( auto& entry : filesystem::directory_iterator(directory_path)) {
        // Skip .mygit directory and other hidden files
        string filename = entry.path().filename().string();
        if (filename[0] == '.' || filename == "mygit") {
            continue;
        }
        
        if (filesystem::is_directory(entry.path())) 
        {
            Tree subtree = buildTree(entry.path());
            tree.addSubtree(entry.path().filename().string(), subtree);
        } 
        else if (filesystem::is_regular_file(entry.path())) 
        {
            ifstream file(entry.path(), ios::binary);
            stringstream buffer;
            buffer << file.rdbuf();
            Blob blob(buffer.str());
            tree.addBlob(entry.path().filename().string(), blob);
        }
    }
    return tree;
}

string handle_write_tree(int flag) 
{
    Tree root_tree = buildTree(filesystem::current_path());
    string root_hash = root_tree.getHash();
    root_tree.writeTree(root_hash);
    if(flag)
    {
        cout << "Tree SHA-1: " << root_hash << endl;
        return "yo";
    }
    else
        return root_hash;
}


void handle_list_tree(vector<string>& commands) 
{
    if (commands.size() < 2) {
        cerr << "Error: SHA-1 hash is required for list-tree command." << endl;
        return;
    }

    string sha1_hash;
    if(commands.size() == 3)
        sha1_hash = commands[2];
    else if(commands.size() == 4)
        sha1_hash = commands[3];
    bool name_only = (commands.size() == 4 && commands[2] == "--name-only");

    if(commands.size() == 4 and commands[2] != "--name-only")
    {
        cerr << "invalid Args" << endl;
        return;
    }
    // Locate the tree object file
    string path = ".mygit/objects/" + sha1_hash.substr(0, 2) + "/" + sha1_hash.substr(2);

    ifstream file(path);
    if (!file) {
        cerr << "Error: Tree object not found for SHA-1 hash: " << sha1_hash << endl;
        return;
    }


    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string type, hash, name;
        ss >> type >> hash >> name;

        if (name_only) {
            cout << name << endl;
        } else {
            string mode = (type == "blob") ? "100644" : "040000"; // file or directory
            cout << mode << " " << type << " " << hash << " " << name << endl;
        }
    }
}