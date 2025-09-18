#include "headers.h"
using namespace std;
#define pb push_back



int main(int argc, char** argv)
{
    vector<string>commands;
    
    for(int i = 0; i < argc; i++)
        commands.pb(argv[i]);
    
    if(commands[1] == "init")
        handle_init();
    else
    {
        if (!filesystem::exists(".mygit")) 
        {
            cerr << ".mygit is not initialised." << endl;
            return 1;
        }
        
        if(commands[1] == "hash-object")
            handle_object_hashing(commands);
        else if(commands[1] == "cat-file")
            handle_cat_file(commands);
        else if(commands[1] == "write-tree")
        {
            if(commands.size() != 2)
            {
                cerr << "Invalid arguments" << endl;
                return -1;
            }
            string a = handle_write_tree(1);
        }
        else if(commands[1] == "ls-tree")
            handle_list_tree(commands);
        else if(commands[1] == "add")
            handle_add_command(commands);
        else if(commands[1] == "commit")
            handle_commit_command(commands);
        else if(commands[1] == "log")
            handle_log_command();
        else if(commands[1] == "checkout")
            handle_checkout_command(commands);
        else
            cout << "Invalid command" << endl;
        
    }
    

}