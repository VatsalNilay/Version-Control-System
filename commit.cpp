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
// class Tree {

// public:
//     map<string, Blob> blobs;
//     map<string, Tree> subtrees;
//     void addBlob(const string& name, const Blob& blob) {
//         blobs[name] = blob;
//     }

//     void addSubtree(const string& name, const Tree& tree) {
//         subtrees[name] = tree;
//     }

//     string getHash()  {
//         string combined_data;
//         for (auto& [name, blob] : blobs) {
//             combined_data += "blob " + blob.getHash() + " " + name + "\n";
//         }
//         for (auto& [name, subtree] : subtrees) {
//             combined_data += "tree " + subtree.getHash() + " " + name + "\n";
//         }
//         return sha1_hash(combined_data);
//     }
//     void writeTree(string hash)  
//     {
//         string path = ".mygit/objects/" + hash.substr(0, 2);
//         if (!filesystem::exists(path)) {
//             filesystem::create_directories(path);
//         }

//         path += "/" + hash.substr(2);

//         ofstream file(path, ios::binary);
//         if (file) {
//             string combined_data;
//             for (auto& [name, blob] : blobs) {
//                 combined_data += "blob " + blob.getHash() + " " + name + "\n";
//             }
//             for (auto& [name, subtree] : subtrees) {
//                 combined_data += "tree " + subtree.getHash() + " " + name + "\n";
//             }
//             file << combined_data;
//             file.close();
//         } else {
//             cerr << "Error: Could not write to file " << path << endl;
//         }
//     }
  
// };
class Commit
{
    public:
        Tree tree;
        Commit* parent;
        string author;
        string message;
        time_t timestamp;
        Commit(const Tree& tree, Commit* parent, const string& author, const string& message)
            : tree(tree), parent(parent), author(author), message(message) 
        {
            timestamp = time(nullptr);
        }

        string getHash() {
            string combined_data = "tree " + tree.getHash() + "\n";
            if (parent) {
            combined_data += "parent " + parent->getHash() + "\n";
            }
            combined_data += "author " + author + "\n";
            combined_data += "message " + message + "\n";
            combined_data += "timestamp " + to_string(timestamp) + "\n";
            return sha1_hash(combined_data);
        }

        void writeCommit(string hash) {
            // string hash = getHash();
            string path = ".mygit/objects/" + hash.substr(0, 2);
            if (!filesystem::exists(path)) {
            filesystem::create_directories(path);
            }

            path += "/" + hash.substr(2);

            ofstream file(path, ios::binary);
            if (file) {
            string combined_data = "tree " + tree.getHash() + "\n";
            if (parent) {
                combined_data += "parent " + parent->getHash() + "\n";
            }
            combined_data += "author " + author + "\n";
            combined_data += "message " + message + "\n";
            combined_data += "timestamp " + to_string(timestamp) + "\n";
            file << combined_data;
            file.close();
            } else {
            cerr << "Error: Could not write to file " << path << endl;
            }
        }

};


string getCurrentTimestamp() 
{
    time_t now = time(nullptr);
    stringstream ss;
    ss << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string getParentCommitSHA() 
{
    ifstream headFile(".mygit/HEAD");
    string parentCommitSHA;
    if (headFile) {
        getline(headFile, parentCommitSHA);
    }
    return parentCommitSHA;
}



Tree buildTreeFromIndex() {
    Tree tree;
    ifstream indexFile(".mygit/index");

    if (!indexFile) {
        cerr << "Error: No files staged for commit." << endl;
        return tree;
    }

    indexFile.seekg(0, ios::end);
    if (indexFile.tellg() == 0) {
        cerr << "Error: Index file is empty. No files staged for commit." << endl;
        return tree;
    }
    indexFile.seekg(0, ios::beg);


    string line;
    while (getline(indexFile, line)) {
        stringstream ss(line);
        string hash, filename;
        ss >> filename >> hash;
        tree.addEntry(filename, hash);
    }

    return tree;
}

void handle_commit_command(vector<string>&commands)
{
    string msg;
    if(commands.size() == 2)
        msg = "Default Commit";
    else
    {
        if(commands[2] != "-m")
        {
            cerr << "Invalid Flag" << endl;
            return;
        }
        if(commands.size() > 4 )
        {
            cerr << "Invalid msg" << endl;
            return;
        }

        msg = commands[3];
    }

    Tree tree = buildTreeFromIndex();
    string treeHash = tree.getHash();
    tree.writeTree();
    string root_hash = handle_write_tree(0);
    //commit metadata
    string timestamp = getCurrentTimestamp();
    string parentCommitSHA = getParentCommitSHA();

    if (!parentCommitSHA.empty()) {
        ifstream parentCommitFile(".mygit/objects/" + parentCommitSHA.substr(0, 2) + "/" + parentCommitSHA.substr(2));
        if (parentCommitFile) {
            string line;
            getline(parentCommitFile, line); // Read the tree line
            string parentTreeHash = line.substr(5); // Extract the tree hash
            if (parentTreeHash == treeHash) {
                cerr << "Nothing new to be added" << endl;
                return;
            }
        }
    }
    string commitMessage = msg;

    //commit data
    stringstream commitData;
    commitData << "tree " << treeHash << "\n";
    commitData << "root_hash " << root_hash << "\n";
    if (!parentCommitSHA.empty()) {
        commitData << "parent " << parentCommitSHA << "\n";
    }
    char* username = getenv("USER");
    if (username == nullptr) {
        cerr << "Error: Could not get the username" << endl;
        return;
    }
    commitData << "author " << username << "\n";
    commitData << "date " << timestamp << "\n";
    commitData << commitMessage << "\n";
    string commitDataStr = commitData.str();
    
    // Calculate commit hash first
    string commitHash = sha1_hash(commitDataStr);

    string commitPath = ".mygit/objects/" + commitHash.substr(0, 2) + "/" + commitHash.substr(2);
    if (filesystem::exists(".mygit/objects/" + commitHash.substr(0, 2))) 
    {
        cerr<<"Nothing new to be added" << endl;
        return;
    }
    filesystem::create_directories(".mygit/objects/" + commitHash.substr(0, 2));

    // Write to log file with commit hash
    ofstream logFile(".mygit/log", ios::app);
    if (logFile) {
        logFile << "commit " << commitHash << "\n";
        logFile << commitDataStr << "\n";
        logFile.close();
    } else {
        cerr << "Error: Could not write to log file" << endl;
    }

    ofstream commitFile(commitPath, ios::binary);
    if (commitFile) 
    {
        commitFile << commitData.str();
        commitFile.close();
        cout << "Commit SHA-1: " << commitHash << endl;
    } 
    else
        cerr << "Error: Could not write commit to " << commitPath << endl;

    //changing head and clearing index file
    ofstream indexFile(".mygit/index", ios::trunc);
    if (!indexFile) {
        cerr << "Error: Could not clear the index file" << endl;
    }
    indexFile.close();
    ofstream headFile(".mygit/HEAD");
    if (headFile) {
        headFile << commitHash;
        headFile.close();
    } else {
        cerr << "Error: Could not update HEAD" << endl;
    }
}
