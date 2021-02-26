#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <list>

void allocateArrayOfCharArrays(char ***array_ptr, size_t array_length, size_t item_size);
void freeArrayOfCharArrays(char **array, size_t array_length);
void splitString(std::string text, char d, char **result);
void printHistory(std::list<std::string> command_history, int history_list_size);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    // `os_path_list` supports up to 16 directories in PATH, 
    //     each with a directory name length of up to 64 characters
    char **os_path_list;
    allocateArrayOfCharArrays(&os_path_list, 16, 128);
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);


    // Example code for how to loop over NULL terminated list of strings
    int i = 0;
    while (os_path_list[i] != NULL)
    {
        printf("PATH[%3d]: %s\n", i, os_path_list[i]);
        i++;
    }

    // Allocate command history doubly-linked list
    std::list<std::string> command_history;
    // TODO: Attempt to load command history from file

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    // Allocate space for input command lists
    // `command_list` supports up to 32 command line parameters, 
    //     each with a parameter string length of up to 128 characters
    char **command_list;
    allocateArrayOfCharArrays(&command_list, 32, 128);

    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    int running = 1;
    std::string user_input;
    while(running) {
        printf("osshell> ");
        std::getline(std::cin, user_input);

        printf("Command recieved.\n");
        // extract command from user input
        splitString(user_input, ' ', command_list);

        printf("Command split.\n");
        
        //std::cout << user_input << "\n";

        std::string command = command_list[0];

        printf("First command: %s\n", command.c_str());

        // check preset commands
        if(command.compare("exit") == 0){
            // 'exit' command
            running = 0;
        } else if(command.compare("history") == 0) {
            // 'history' command

            if(command_list[1] != NULL) {
                printf("Checking arg\n");
                std::string argument = command_list[1];
                if(argument.compare("clear") == 0) {
                    // TODO: Make this work
                    printf("History being cleared...\n");
                    // argument is `clear`, i.e. `history clear`
                    // empty out the command history
                    while(command_history.size() > 0) {
                        printf("Popping args...\n");
                        command_history.pop_back();
                    }
                } else {
                    // parse to int
                    // TODO: Do this
                }
            } else {
                int history_list_size = command_history.size();
                printHistory(command_history, history_list_size);
            }


            // loop through each entry in command_history and print.
        } else {
            // not a pre-defined command

            // check if searching for a local command
            if(command.substr(0,2).compare("./") == 0) {
                // command is a local executable
                // TODO: Implement local command execution
            } else {
                // command is a global executable, check path
                // TODO: Implement command path search
                // search PATH folders for matching file.
            }
        }

        // add the command string to history
        // TODO: Append command string to history
        command_history.push_front(user_input);
        while(command_history.size() > 128) {
            command_history.pop_back();
        }

    }

    // Free allocated memory
    freeArrayOfCharArrays(os_path_list, 16);
    freeArrayOfCharArrays(command_list, 32);

    return 0;
}

/*
   array_ptr: pointer to list of strings to be allocated
   array_length: number of strings to allocate space for in the list
   item_size: length of each string to allocate space for
*/
void allocateArrayOfCharArrays(char ***array_ptr, size_t array_length, size_t item_size)
{
    int i;
    *array_ptr = new char*[array_length];
    for (i = 0; i < array_length; i++)
    {
        (*array_ptr)[i] = new char[item_size];
    }
}

/*
   array: list of strings to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        delete[] array[i];
    }
    delete[] array;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: NULL terminated list of strings (char **) - result will be stored here
*/
void splitString(std::string text, char d, char **result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::vector<std::string> list;
    std::string token;
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
                    list.push_back(token);
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
                    list.push_back(token);
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
        list.push_back(token);
    }

    for (i = 0; i < list.size(); i++)
    {
        strcpy(result[i], list[i].c_str());
    }
    //result[list.size()] = NULL;
}

void printHistory(std::list<std::string> command_history, int history_list_size) {
    int history_index = 0;
    for(std::list<std::string>::reverse_iterator it = command_history.rbegin(); it != command_history.rend(); ++it){
        history_index += 1;
        std::string cmd = (*it);
        std::cout << "  " << history_index << ": " << cmd << '\n';
    }
}
