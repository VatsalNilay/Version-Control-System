#include "headers.h"
using namespace std;


class Tree {
    map<string, string> entries; // filename -> blob SHA-1

public:
    void addEntry(const string& name, const string& hash) {
        entries[name] = hash;
    }

    string getHash() const {
        string combined_data;
        for (const auto& [name, hash] : entries) {
            combined_data += "blob " + hash + " " + name + "\n";
        }
        return sha1_hash(combined_data);
    }
    void addSubtree(const string& name, const Tree& subtree) {
        entries[name] = "tree " + subtree.getHash();
    }
    void writeTree() const {
        string treeHash = getHash();
        string path = ".mygit/objects/" + treeHash.substr(0, 2) + "/" + treeHash.substr(2);

        if (!filesystem::exists(path)) {
            filesystem::create_directories(".mygit/objects/" + treeHash.substr(0, 2));
        }

        ofstream file(path, ios::binary);
        if (file) {
            for (const auto& [name, hash] : entries) {
                file << "blob " << hash << " " << name << "\n";
            }
        }
    }
};


void extract_tree(const string& tree_sha, const string& path) {
        string tree_path = ".mygit/objects/" + tree_sha.substr(0, 2) + "/" + tree_sha.substr(2);

        if (!filesystem::exists(tree_path)) {
            cerr << "Tree object not found: " << tree_sha << endl;
            return;
        }

        ifstream tree_file(tree_path, ios::binary);
        if (!tree_file) {
            cerr << "Failed to open tree file: " << tree_sha << endl;
            return;
        }

        string line;
        while (getline(tree_file, line)) {
            istringstream iss(line);
            string type, sha, name;
            if (!(iss >> type >> sha >> name)) {
                cerr << "Malformed tree entry: " << line << endl;
                continue;
            }

            string full_path = path + "/" + name;
            if (type == "blob") {
                string blob_path = ".mygit/objects/" + sha.substr(0, 2) + "/" + sha.substr(2);
                ifstream blob_file(blob_path, ios::binary);
                if (!blob_file) {
                    cerr << "Failed to open blob file: " << sha << endl;
                    continue;
                }

                // Remove existing file if it exists before creating new one
                if (filesystem::exists(full_path)) {
                    filesystem::remove(full_path);
                }

                ofstream out_file(full_path, ios::binary);
                if (!out_file) {
                    cerr << "Failed to create file: " << full_path << endl;
                    continue;
                }

                out_file << blob_file.rdbuf();
                cout << "Updated: " << full_path << endl;
            } else if (type == "tree") {
                filesystem::create_directories(full_path);
                extract_tree(sha, full_path);
            }
        }
    }

void handle_checkout_command(vector<string>&commands)
{
    if(commands.size() != 3)
    {
        cerr << "Invalid arguments" << endl;
        return;
    }
    string  commit_id = commands[2];

    string commit_path = ".mygit/objects/" + commit_id.substr(0, 2) + "/" + commit_id.substr(2);

    if (!filesystem::exists(commit_path)) {
        cerr << "Commit ID not found" << endl;
        return;
    }

    ifstream commit_file(commit_path, ios::binary);
    if (!commit_file) {
        cerr << "Failed to open commit file" << endl;
        return;
    }

    string line;
    string tree_sha;
    
    // Read the first line which contains "tree <hash>"
    if (getline(commit_file, line)) {
        if (line.substr(0, 5) == "tree ") {
            tree_sha = line.substr(5);
        } else {
            cerr << "Invalid commit format" << endl;
            return;
        }
    } else {
        cerr << "Empty commit file" << endl;
        return;
    }
    string tree_path = ".mygit/objects/" + tree_sha.substr(0, 2) + "/" + tree_sha.substr(2);

    if (!filesystem::exists(tree_path)) {
        cerr << "Tree object not found: " << tree_sha << endl;
        return;
    }

    ifstream tree_file(tree_path, ios::binary);
    if (!tree_file) {
        cerr << "Failed to open tree file" << endl;
        return;
    }
    // Don't remove all files - instead, only update files that exist in the commit
    // and optionally warn about untracked files
    
    extract_tree(tree_sha, ".");

    
}