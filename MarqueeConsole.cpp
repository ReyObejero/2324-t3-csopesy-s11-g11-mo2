#include "MarqueeConsole.h"
#include "ConsoleManager.h"
#include "MarqueeWorkerThread.h"
#include <string>
#include "Commands.h"
#include <thread>
#include <mutex>

std::mutex console_mutex;

MarqueeConsole::MarqueeConsole(int polling_rate) : running_(true) {
    this->polling_rate_ = polling_rate;
    this->bottom_pad_ = 0;
}

void MarqueeConsole::UpdateInputLog(const std::string& input) {
    input_log_.push_back(input);
    bottom_pad_ = input_log_.size();
}

void MarqueeConsole::RedrawInputPrompt(ConsoleManager& console_manager, const std::string& input_prompt, const std::string& input_command, int pos_y) {
    std::lock_guard<std::mutex> lock(console_mutex);
    console_manager.WriteAtPosition(input_prompt + input_command, 0, pos_y);
}

void MarqueeConsole::RedrawLog(ConsoleManager& console_manager, int display_window_height) {
    std::lock_guard<std::mutex> lock(console_mutex);
    int pad = bottom_pad_;
    int index = 0;

    for (int i = input_log_.size() - 1; i >= 0 ; i--) {
        console_manager.WriteAtPosition("Command \"" + input_log_[index] + "\" processed in " + console_manager.getCurrentConsoleName(), 0, display_window_height - pad);

        index++;

        if (pad >= 0) {
            pad--;
        }
    }
}

void MarqueeConsole::Run() {
    MarqueeWorkerThread* marquee_worker_thread = new MarqueeWorkerThread(10, "Hello world in marquee!");
    marquee_worker_thread->Start();
    ConsoleManager console_manager;
    char key_pressed;
    std::string input_prompt = "Enter a command for MARQUEE_CONSOLE: ";
    std::string input_command = "";
    int display_window_height = console_manager.GetDisplayWindowBounds().Bottom;

    while (running_) {
        key_pressed = 0;
        RedrawInputPrompt(console_manager, input_prompt, input_command, display_window_height - bottom_pad_ - 1);
        RedrawLog(console_manager, display_window_height);
        console_manager.PollKeyboard(key_pressed);
        std::this_thread::sleep_for(std::chrono::milliseconds(polling_rate_));

        if (key_pressed == '\r') {
            if (input_command == "exit") {
                running_ = false;
                marquee_worker_thread->Stop();
                delete marquee_worker_thread;

                system("cls");
                // Find the console with the name "MAIN_MENU"
                for (const auto& consolePtr : console_manager.getConsoles()) {
                    if (consolePtr->getName() == "MAIN_MENU") {
                        console_manager.setCurrentConsole(consolePtr);
                        break;
                    }
                }
                // If "MAIN_MENU" console is found, draw its main content
                if (console_manager.getCurrentConsole() && console_manager.getCurrentConsoleName() == "MAIN_MENU") {
                    displayHeader();
                }
            }
            else {
                UpdateInputLog(input_command);
                marquee_worker_thread->SetBoundsBottomPad(bottom_pad_ + 2);
            }
            input_command = "";
        }
        else if (key_pressed == 127 || key_pressed == 8) {
            if (!input_command.empty()) {
                input_command.pop_back();
            }
        }
        else if (key_pressed != 0) {
            input_command += key_pressed;
        }
    }
}
