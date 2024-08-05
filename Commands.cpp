#include "Commands.h"
#include "Config.h"
#include "ConsoleManager.h"
#include "MarqueeConsole.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

void displayHeader() {
    std::cout << "  ____ ____   ___  ____  _____ ______   __\n"
        " / ___/ ___| / _ \\|  _ \\| ____/ ___\\ \\ / /\n"
        "| |   \\___ \\| | | | |_) |  _| \\___ \\\\ V / \n"
        "| |___ ___) | |_| |  __/| |___ ___) || |  \n"
        " \\____|____/ \\___/|_|   |_____|____/ |_|  \n";

    std::cout << "\033[1;32m"; // 1;32m represents bold green text
    std::cout << "Hi, Welcome to the CSOPESY command line!" << std::endl;

    std::cout << "\033[1;33m"; // 1;33m represents bold yellow text
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
    std::cout << "\033[0m"; // Reset text color to default
}

void clearScreen() {
    system("cls");
    displayHeader();
}

/*bool handleCommand(const std::string& command, ConsoleManager& consoleManager) {
    if (command != "initialize" && command != "exit") {
        if (Config::GetConfigParameters().num_cpu == NULL ||
            Config::GetConfigParameters().scheduler == NULL ||
            Config::GetConfigParameters().quantum_cycles == NULL ||
            Config::GetConfigParameters().preemptive == NULL ||
            Config::GetConfigParameters().batch_process_freq == NULL ||
            Config::GetConfigParameters().min_ins == NULL ||
            Config::GetConfigParameters().max_ins == NULL ||
            Config::GetConfigParameters().delay_per_exec == NULL) {
            std::cout << "Initialize the program with command \"initialize\"" << std::endl;

            return true;
        }
    }

    std::vector<std::shared_ptr<Console>> consoles = consoleManager.getConsoles();
    // Handle user input

    // tokenize user input
    std::stringstream ss(command);
    std::vector<std::string> tokens;
    std::string token;

    while (ss >> token) {
        tokens.push_back(token);
    }

    // Current console is not the main menu and the exit command is entered.
    if (command == "exit" && (consoleManager.getCurrentConsoleName() != "MAIN_MENU")) {
        system("cls");
        // Find the console with the name "MAIN_MENU"
        for (const auto& consolePtr : consoles) {
            if (consolePtr->getName() == "MAIN_MENU") {
                consoleManager.setCurrentConsole(consolePtr);
                break;
            }
        }
        // If "MAIN_MENU" console is found, draw its main content
        if (consoleManager.getCurrentConsole() && consoleManager.getCurrentConsoleName() == "MAIN_MENU") {
            displayHeader();
        }
    }

    // Current console is the main menu and the exit command is entered.
    else if (command == "exit" && consoleManager.getCurrentConsoleName()  == "MAIN_MENU") {
        return false;
    }

    // Current console is the main menu and the screen -s (create process) is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-s" && tokens[2] != "" && consoleManager.getCurrentConsoleName() == "MAIN_MENU") {
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
            consoleManager.addConsole(newConsole);

            consoleManager.setCurrentConsole(newConsole);
            consoleManager.getCurrentConsole()->drawProcess();
        }

        //currentConsole = new Console(tokens[2], 0, 100, 50);

        //currentConsole->drawProcess();

    }

    else if (command == "show") {
        consoleManager.drawAllConsoles();
    }

    // Current console is the main menu and the the screen -r (access process screen) command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r" && tokens[2] != "" && consoleManager.getCurrentConsoleName() == "MAIN_MENU") {

        for (const auto& consolePtr : consoles) {

            if (consolePtr->getName() == tokens[2]) {
                consoleManager.setCurrentConsole(consolePtr);
                break;
            }
        }

        if (consoleManager.getCurrentConsole() && consoleManager.getCurrentConsoleName() == tokens[2]) {
            consoleManager.getCurrentConsole()->drawProcess();
        }
        else {
            std::cout << "Can't find screen" << std::endl;
        }

    }

    // The current console is not the main menu and the screen -s command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-s" && tokens[2] != "" && consoleManager.getCurrentConsoleName() != "MAIN_MENU") {

        std::cout << "Unkown Command" << std::endl;
    }

    // The current ocnsole is not the main menu and the screen -r command is entered.
    else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r" && tokens[2] != "" && consoleManager.getCurrentConsoleName() != "MAIN_MENU") {

        std::cout << "Unkown Command" << std::endl;
    }
    else if (command == "clear") {
        clearScreen();
    }
    else if (command == "initialilze") {}
    else if (command == "marquee") {
        MarqueeConsole marqueeConsole(10);
        marqueeConsole.Run();
    }
    else if (command == "report-util") {}
    else if (command == "scheduler-stop") {}
    else if (command == "scheduler-test") {}
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
    else if (tokens.size() == 2 && tokens[0] == "screen" && tokens[1] == "-ls" && consoleManager.getCurrentConsoleName() == "MAIN_MENU") {
        const int num_processes = 10;
        const int commands_per_process = 100;
        const int num_cores = 4;
        Scheduler scheduler(num_cores);
        scheduler.start();

        for (int i = 1; i <= num_processes; ++i) {
            scheduler.add_process(new Process("process" + std::to_string(i), commands_per_process));
        }

        scheduler.screen_ls();

        std::string command;
        while (true) {
            std::cout << "Enter command: ";
            std::cin >> command;


            if (command == "screen-ls") {
                scheduler.screen_ls();
            }
            else if (command == "exit") {
                scheduler.stop();
                break;
            }
        }
    }
    else if (command == "initialize") {
        Config::Initialize();
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
    }

    return true;
}*/