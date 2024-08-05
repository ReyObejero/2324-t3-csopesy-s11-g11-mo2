#include "ConsoleManager.h"
#include <string>
#include <vector>
#include <atomic>


class MarqueeConsole {
public:
    MarqueeConsole(int polling_rate);
    void Run();

private:
    void UpdateInputLog(const std::string& input);
    void RedrawInputPrompt(ConsoleManager& console_manager, const std::string& input_prompt, const std::string& input_command, int pos_y);
    void RedrawLog(ConsoleManager& console_manager, int display_window_height);

    int polling_rate_;
    std::vector<std::string> input_log_;
    int bottom_pad_;
    std::atomic<bool> running_;
};
