#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <list>
#include <filesystem>
#include <sys/wait.h>
#include <fstream>

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);
void printHistory(std::list<std::string> command_history, int history_list_size);
bool isProgram(std::string name, std::string folder);
bool isInPath(std::string name, std::vector<std::string> path_list, std::string* pathLocation);
void runProgram(std::string command, std::string path, char*** command_list_exec);
void loadHistory(std::list<std::string> *history);
void saveHistory(std::list<std::string> *history);
bool isNumeric(std::string input);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    // Allocate command history doubly-linked list
    std::list<std::string> command_history;
    // TODO: Attempt to load command history from file
    loadHistory(&command_history);

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; // to store command user types in, split into its variour parameters
    char **command_list_exec; // command_list converted to an array of character arrays
    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    int running = 1;
    bool flag_addToHistory;
    std::string user_input;
    while(running) {
        printf("osshell> ");
        std::getline(std::cin, user_input);

        // set flags
        flag_addToHistory = true;

        // extract command from user input
        splitString(user_input, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);

        std::string command = command_list[0];

        // check preset commands
        if(command.compare("exit") == 0){
            // 'exit' command
            printf("\n");
            running = 0;
        } else if(command.compare("history") == 0) {
            // 'history' command

            if(command_list.size() > 1) {
                std::string argument = command_list.at(1);
                if(argument.compare("clear") == 0) {
                    // argument is `clear`, i.e. `history clear`
                    // empty out the command history
                    while(command_history.size() > 0) {
                        command_history.pop_back();
                    }
                    flag_addToHistory = false;
                } else {
                    // parse to int
                    if(isNumeric(argument)) {
                        int num = std::stoi(argument);
                        if(num <= 0) {
                            printf("Error: history expects an integer > 0 (or 'clear')\n");
                        } else {
                            printHistory(command_history, num);
                        }
                    } else {
                        printf("Error: history expects an integer > 0 (or 'clear')\n");
                    }
                }
            } else {
                int history_list_size = (int)command_history.size();
                printHistory(command_history, history_list_size);
            }
        } else {
            // not a pre-defined command
            // check if searching for a local command
            if(command.substr(0,2).compare("./") == 0) {
                // command is a local executable
                if(isProgram(command, "")) {
                    runProgram(command, "", &command_list_exec);
                } else {
                    printf("%s: Error command not found\n", command.c_str());
                }
            } else {
                // command is a global executable, check path
                // search PATH folders for matching file.
                std::string pathLocation;
                if(isInPath(command, os_path_list, &pathLocation)) {
                    runProgram(command, pathLocation, &command_list_exec);
                } else {
                    printf("%s: Error command not found\n", command.c_str());
                }
            }
        }

        // add the command string to history
        if(flag_addToHistory){
            command_history.push_front(user_input);
            while(command_history.size() > 128) {
                command_history.pop_back();
            }
        }
    }
    saveHistory(&command_history);
    return 0;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
}

void printHistory(std::list<std::string> command_history, int history_list_size) {
    int print_marker = (int)command_history.size() - history_list_size;
    int history_index = 0;
    for(std::list<std::string>::reverse_iterator it = command_history.rbegin(); it != command_history.rend(); ++it){
        history_index += 1;
        if (history_index > print_marker){
            std::string cmd = (*it);
            std::cout << "  " << history_index << ": " << cmd << '\n';
        }
    }
}

bool isProgram(std::string name, std::string folder) {
    std::string pathString = folder + name;
    if(std::filesystem::is_regular_file(pathString)) {
        if(!access(pathString.c_str(), X_OK)) {
            return true;
        }
    }
    return false;
}

bool isInPath(std::string name, std::vector<std::string> path_list, std::string* pathLocation) {
    for(int i = 0; i < path_list.size(); i++) {
        std::string pathFolder = path_list.at(i) + "/";
        if(isProgram(name, pathFolder)) {
            (*pathLocation) = pathFolder;
            return true;
        }
    }
    return false;
}

void runProgram(std::string command, std::string path, char*** command_list_exec) {
    pid_t pid = fork();

    if(pid == 0) {
        // child
        std::string cmdv = path + command;
        execv(cmdv.c_str(), *command_list_exec);
        exit(0);
    } else if(pid == -1) {
        // failed
        printf("ERROR: Unable to fork.\n");
        exit(EXIT_FAILURE);
    } else {
        // parent
        waitpid(pid, NULL, 0);
        return;
    }
}

void loadHistory(std::list<std::string> *history) {
    std::ifstream file;
    file.open("history");
    std::string line;
    if(file.is_open()) {
        while(std::getline(file, line)) {
            (*history).push_front(line);
        }
        file.close();
    }
    return;
}

void saveHistory(std::list<std::string> *history) {
    std::ofstream file;
    file.open("history");
    for(std::list<std::string>::reverse_iterator it = (*history).rbegin(); it != (*history).rend(); ++it){
        std::string cmd = (*it);
        file << cmd << "\n";
    }
    file.close();
    return;
}

bool isNumeric(std::string input) {
    for(int i = 0; i < input.size(); i++) {
        int cvar = (int)input.at(i);
        if(cvar < 48 || cvar > 57) {
            return false;
        }
    }
    return true;
}