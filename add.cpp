#include "headers.h"
using namespace std;

void remove_existing_entry(const string& fileName) 
{
    ifstream indexFile(".mygit/index");
    if (!indexFile.is_open()) {
        cerr << "Error: Could not open index file." << endl;
        return;
    }

    vector<string> lines;
    string line;
    while (getline(indexFile, line)) {
        if (line.substr(0, line.find(' ')) != fileName) {
            lines.push_back(line);
        }
    }
    indexFile.close();

    ofstream outFile(".mygit/index", ios::trunc);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open index file." << endl;
        return;
    }

    for (const auto& l : lines) {
        outFile << l << endl;
    }
    outFile.close();
}
void add_to_staging_area(vector<string>& files) 
{
    ofstream indexFile(".mygit/index", ios::app);
    if (!indexFile.is_open()) {
        cerr << "Error: Could not open index file." << endl;
        return;
    }

    for (auto& file : files) {
        ifstream inFile(file, ios::binary);
        if (!inFile.is_open()) {
            cerr << "Error: Could not open file " << file << endl;
            continue;
        }

        stringstream buffer;
        buffer << inFile.rdbuf();
        string fileContent = buffer.str();
        string fileSHA = sha1_hash(fileContent); 

        // Create blob object file
        string blobDir = ".mygit/objects/" + fileSHA.substr(0, 2);
        string blobPath = blobDir + "/" + fileSHA.substr(2);
        
        if (!filesystem::exists(blobDir)) {
            filesystem::create_directories(blobDir);
        }
        
        if (!filesystem::exists(blobPath)) {
            ofstream blobFile(blobPath, ios::binary);
            if (blobFile) {
                blobFile << fileContent;
                blobFile.close();
            } else {
                cerr << "Error: Could not write blob file " << blobPath << endl;
                continue;
            }
        }

        remove_existing_entry(file); // Remove existing entry before adding new one

        indexFile << file << " " << fileSHA << endl;
    }

    indexFile.close();
}
void handle_add_command(vector<string>& commands) 
{
    if (commands.size() < 2) {
        cerr << "Error: No files specified for adding." << endl;
        return;
    }

    vector<string> filesToAdd;
    if (commands[2] == ".") 
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(".")) 
        {
            string filepath = entry.path().string();
            string filename = entry.path().filename().string();
            
            // Skip .mygit directory, mygit executable, and other hidden files
            if (filepath.substr(0,3) == "./.") {
                continue;
            }
            if (filename == "mygit") {
                continue;
            }
            if (entry.is_regular_file()) 
            {
                filesToAdd.push_back(filepath);
            }
        }
    } else {
        for (size_t i = 2; i < commands.size(); ++i) {
            filesToAdd.push_back(commands[i]);
        }
    }

    add_to_staging_area(filesToAdd);
}


