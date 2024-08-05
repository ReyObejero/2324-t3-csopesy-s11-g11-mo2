#pragma once

#include "Console.h"
#include <string>
#include <vector>
#include <memory>
#include <windows.h>

class ConsoleManager {
private:
    static std::shared_ptr<Console> currentConsole;
    static std::shared_ptr<Console> previousConsole;
    static std::vector<std::shared_ptr<Console>> consoles;
    HANDLE console_handle_;
    CONSOLE_SCREEN_BUFFER_INFO csbi_info_;

public:
    ConsoleManager();
    ~ConsoleManager();

    void addConsole(const std::shared_ptr<Console>& console);
    void drawAllConsoles();
    std::vector<std::shared_ptr<Console>> getConsoles();
    std::shared_ptr<Console> getCurrentConsole();
    static std::string getCurrentConsoleName();
    void setCurrentConsole(std::shared_ptr<Console> console);
    COORD GetCursorPosition();
    SMALL_RECT GetDisplayWindowBounds();
    void ClearConsole();
    void PollKeyboard(char& key_pressed);
    void SetCursorPosition(int pos_x, int pos_y);
    void WriteAtPosition(const std::string& text, int pos_x, int pos_y);
    bool handleCommand(const std::string& command);
};
