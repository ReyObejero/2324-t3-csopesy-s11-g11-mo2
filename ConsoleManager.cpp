#include "ConsoleManager.h"
#include "Console.h"
#include "Config.h"
#include "Commands.h"
#include "MarqueeConsole.h"
#include <iostream>
#include "Process.h";
#include <string>
#include <vector>
#include <conio.h>
#include <random>

std::shared_ptr<Console> ConsoleManager::currentConsole = nullptr;
std::shared_ptr<Console> ConsoleManager::previousConsole = nullptr;
std::vector<std::shared_ptr<Console>> ConsoleManager::consoles;
int totalCount = 0;

ConsoleManager::ConsoleManager() : console_handle_(GetStdHandle(STD_OUTPUT_HANDLE)) {
    bool mainMenuExists = false;
    bool marqueeConsoleExists = false;

    // Check if MAIN_MENU console already exists
    for (const auto& consolePtr : consoles) {
        if (consolePtr->getName() == "MAIN_MENU") {
            mainMenuExists = true;
            break;
        }
    }

    // Create MAIN_MENU console if it doesn't exist
    if (!mainMenuExists) {
        std::shared_ptr<Console> mainConsole(new Console("MAIN_MENU", 1, 1, 0));
        consoles.push_back(mainConsole);
        if (currentConsole == nullptr) {
            currentConsole = mainConsole;
        }
    }

    // Check if MARQUEE_CONSOLE already exists
    for (const auto& consolePtr : consoles) {
        if (consolePtr->getName() == "MARQUEE_CONSOLE") {
            marqueeConsoleExists = true;
            break;
        }
    }

    // Create MARQUEE_CONSOLE if it doesn't exist
    if (!marqueeConsoleExists) {
        std::shared_ptr<Console> marqueeConsole(new Console("MARQUEE_CONSOLE", 0, 0, 1));
        consoles.push_back(marqueeConsole);
    }

    GetConsoleScreenBufferInfo(console_handle_, &csbi_info_);
}


bool ConsoleManager::handleCommand(const std::string& command) {
    // Initialize prompt
    if (command != "initialize" && command != "exit") {
        if (Config::GetConfigParameters().num_cpu == NULL ||
            Config::GetConfigParameters().scheduler == "" ||
            Config::GetConfigParameters().quantum_cycles == NULL ||
            Config::GetConfigParameters().preemptive == NULL ||
            Config::GetConfigParameters().batch_process_freq == NULL ||
            Config::GetConfigParameters().min_ins == NULL ||
            Config::GetConfigParameters().max_ins == NULL ||
            Config::GetConfigParameters().delay_per_exec == NULL) {
            std::cout << "Initialize the program with command \"initialize\"" << std::endl;
        }
    }

    std::vector<std::shared_ptr<Console>> consoles = getConsoles();
    // Handle user input

    // tokenize user input
    std::stringstream ss(command);
    std::vector<std::string> tokens;
    std::string token;

    while (ss >> token) {
        tokens.push_back(token);
    }

    // Current console is not the main menu and the exit command is entered.
    if (command == "exit" && (getCurrentConsoleName() != "MAIN_MENU")) {
        system("cls");
        // Find the console with the name "MAIN_MENU"
        for (const auto& consolePtr : consoles) {
            if (consolePtr->getName() == "MAIN_MENU") {
                setCurrentConsole(consolePtr);
                break;
            }
        }
        // If "MAIN_MENU" console is found, draw its main content
        if (getCurrentConsole() && getCurrentConsoleName() == "MAIN_MENU") {
            displayHeader();
        }
    }

    // Current console is the main menu and the exit command is entered.
    else if (command == "exit" && getCurrentConsoleName() == "MAIN_MENU") {
        return false;
    }

    // Current console is the main menu and the screen -s (create process) is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-s" && tokens[2] != "" && getCurrentConsoleName() == "MAIN_MENU") {
        // go to the next screen
        // add this to the console
        //console = new Console(tokens[2], 0, 50, 32332);
        //addConsole(&console);
        // first check if the name is already existing
        std::cout << "screen -s processing";
        bool isExistingProcess = false;
        for (const auto& consolePtr : consoles) {

            if (consolePtr->getName() == tokens[2]) {
                std::cout << "Please choose another name.";
                isExistingProcess = true;
                break;
            }
        }

        if (!isExistingProcess) {
            std::shared_ptr<Console> newConsole(new Console(tokens[2], 0, 50, 1234));
            addConsole(newConsole);

            setCurrentConsole(newConsole);
            getCurrentConsole()->drawProcess();
        }

    }

    else if (command == "show") {
        drawAllConsoles();
    }

    // Current console is the main menu and the the screen -r (access process screen) command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r" && tokens[2] != "" && getCurrentConsoleName() == "MAIN_MENU") {

        for (const auto& consolePtr : consoles) {

            if (consolePtr->getName() == tokens[2]) {
                setCurrentConsole(consolePtr);
                break;
            }
        }

        if (getCurrentConsole() && getCurrentConsoleName() == tokens[2]) {
            getCurrentConsole()->drawProcess();
        }
        else {
            std::cout << "Can't find screen" << std::endl;
        }

    }

    // The current console is not the main menu and the screen -s command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-s" && tokens[2] != "" && getCurrentConsoleName() != "MAIN_MENU") {

        std::cout << "Unkown Command" << std::endl;
    }

    // The current ocnsole is not the main menu and the screen -r command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r" && tokens[2] != "" && getCurrentConsoleName() != "MAIN_MENU") {

        std::cout << "Unkown Command" << std::endl;
    }
    else if (command == "clear") {
        clearScreen();
    }
    else if (command == "initialilze") {}
    else if (command == "marquee") {
        MarqueeConsole marqueeConsole(10);
        
        for (const auto& consolePtr : consoles) {
            if (consolePtr->getName() == "MARQUEE_CONSOLE") {
                setCurrentConsole(consolePtr);
                break;
            }
        }

        marqueeConsole.Run();
    }

    else if (command == "screen") {
        // Implement screen function
        std::cout << "'screen' command recognized. Doing something." << std::endl;
    }
    else if (command == "process-smi") {
        // Implement process-smi function
        std::cout << "'process-smi' command recognized. Doing something." << std::endl;
    }
    else if (command == "nvidia-smi") {
        // Implement nvidia-smi function
        std::cout << "'nvidia-smi' command recognized. Doing something." << std::endl;
    }

    else {
        std::cout << "Unknown command: " << command << std::endl;
    }

    return true;
}

void ConsoleManager::addConsole(const std::shared_ptr<Console>& console) {
    consoles.push_back(console);
}

ConsoleManager::~ConsoleManager() {}

std::string ConsoleManager::getCurrentConsoleName() {
    if (currentConsole) {
        return currentConsole->getName();
    }
    else {
        return "MAIN_MENU";
    }
}

void ConsoleManager::setCurrentConsole(std::shared_ptr<Console> console)
{
    currentConsole = console;
}

void ConsoleManager::drawAllConsoles() {
   
    for (const auto& consolePtr : consoles) {
        Console& console = *consolePtr;

        // Now you can access properties of the console object
        std::cout << "Name: " << console.getName() << std::endl;
        
        //consolePtr->drawProcess();
    }

    std::cout << "Currently Selected Console: " << std::endl;
    std::cout << currentConsole->getName();
}

std::vector<std::shared_ptr<Console>> ConsoleManager::getConsoles()
{
    return this->consoles;
}

std::shared_ptr<Console> ConsoleManager::getCurrentConsole()
{
    return this->currentConsole;
}

COORD ConsoleManager::GetCursorPosition() {
    return csbi_info_.dwCursorPosition;
}

SMALL_RECT ConsoleManager::GetDisplayWindowBounds() {
    return csbi_info_.srWindow;
}

void ConsoleManager::ClearConsole() {
    SMALL_RECT scroll_rect = { 0, 0, csbi_info_.srWindow.Right, csbi_info_.srWindow.Bottom };
    COORD scroll_target = { 0, (SHORT)(0 - csbi_info_.dwSize.Y) };
    CHAR_INFO fill = { { TEXT(' ') },  csbi_info_.wAttributes };
    ScrollConsoleScreenBuffer(console_handle_, &scroll_rect, NULL, scroll_target, &fill);
}

void ConsoleManager::PollKeyboard(char& key_pressed) {
    if (_kbhit()) {
        key_pressed = _getch();
    }
}


void ConsoleManager::SetCursorPosition(int pos_x, int pos_y) {
    COORD coord = { pos_x, pos_y };
    SetConsoleCursorPosition(console_handle_, coord);
}

void ConsoleManager::WriteAtPosition(const std::string& text, int pos_x, int pos_y) {
    SetCursorPosition(pos_x, pos_y);
    WriteConsoleA(console_handle_, text.c_str(), text.size(), NULL, NULL);
}
