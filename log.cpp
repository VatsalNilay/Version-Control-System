#include "headers.h"
using namespace std;

void handle_log_command()
{
    ifstream headFile(".mygit/HEAD");
    if (!headFile) {
        cerr << "No commits found (HEAD file not found)" << endl;
        return;
    }
    
    string currentCommitSHA;
    if (!getline(headFile, currentCommitSHA) || currentCommitSHA.empty()) {
        cerr << "No commits found (HEAD is empty)" << endl;
        return;
    }
    headFile.close();
    
    // Traverse commit history from HEAD backwards
    while (!currentCommitSHA.empty()) {
        string commitPath = ".mygit/objects/" + currentCommitSHA.substr(0, 2) + "/" + currentCommitSHA.substr(2);
        
        ifstream commitFile(commitPath);
        if (!commitFile) {
            cerr << "Error: Could not read commit " << currentCommitSHA << endl;
            break;
        }
        
        // Display commit header
        cout << "Commit: " << currentCommitSHA << endl;
        
        string line;
        string parentSHA = "";
        
        // Parse and display commit data
        while (getline(commitFile, line)) {
            if (line.substr(0, 5) == "tree ") {
                // Skip tree line for display
                continue;
            } else if (line.substr(0, 10) == "root_hash ") {
                // Skip root_hash line for display  
                continue;
            } else if (line.substr(0, 7) == "parent ") {
                parentSHA = line.substr(7);
                cout << "Parent: " << parentSHA << endl;
            } else if (line.substr(0, 7) == "author ") {
                cout << "Author: " << line.substr(7) << endl;
            } else if (line.substr(0, 5) == "date ") {
                cout << "Date: " << line.substr(5) << endl;
            } else if (!line.empty()) {
                // This is the commit message
                cout << "Message: " << line << endl;
            }
        }
        
        cout << endl; // Add blank line between commits
        commitFile.close();
        
        // Move to parent commit for next iteration
        currentCommitSHA = parentSHA;
    }
}