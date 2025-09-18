#include "headers.h"
using namespace std;
namespace fs = std::filesystem;

void handle_init()
{
    cout << "Initializing Repository ..." << endl;
    bool isDirPresent = fs::exists(".mygit");
    if(isDirPresent) {
        cout << "Repository already exists !!" << endl;
        return;
    }
    if (!fs::create_directories(".mygit/objects") ||
        !fs::create_directories(".mygit/commits") ||
        !fs::create_directories(".mygit/stage") ||
        !fs::create_directories(".mygit/logs")) {
        cerr << red << "Error: Failed to create one or more directories" << reset << endl;
        return;
    }
    vector<string> addArgs = {"add","."};
    handle_add_command(addArgs);
}