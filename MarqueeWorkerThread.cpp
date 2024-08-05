#include "MarqueeWorkerThread.h"
#include "ConsoleManager.h"
#include <mutex>
#include <chrono>

extern std::mutex console_mutex;

MarqueeWorkerThread::MarqueeWorkerThread(int refresh_rate, std::string marquee_text)
    : bounds_bottom_pad_(2), min_marquee_pos_x_(0), min_marquee_pos_y_(4), dx_(3), dy_(1), running_(false) {
    this->refresh_rate_ = refresh_rate;
    this->marquee_text_ = marquee_text;
}

void MarqueeWorkerThread::Start() {
    running_ = true;
    worker_thread_ = std::thread(&MarqueeWorkerThread::Run, this);
}

void MarqueeWorkerThread::Stop() {
    running_ = false;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

int MarqueeWorkerThread::GetBoundsBottomPad() {
    return bounds_bottom_pad_;
}

void MarqueeWorkerThread::SetBoundsBottomPad(int pad) {
    bounds_bottom_pad_ = pad;
}

bool MarqueeWorkerThread::IsInBounds(int text_length, int pos_x, int pos_y, int display_window_width, int display_window_height) {
    return (pos_x + text_length <= display_window_width) && (pos_y <= display_window_height);
}

void MarqueeWorkerThread::CursorIncrementText(const std::string& text, int& current_pos_x, int& current_pos_y) {
    ConsoleManager console_manager;
    int display_window_width = console_manager.GetDisplayWindowBounds().Right;
    int display_window_height = console_manager.GetDisplayWindowBounds().Bottom;
    console_manager.WriteAtPosition(marquee_text_, current_pos_x, current_pos_y);

    if (current_pos_x + dx_ < min_marquee_pos_x_ || current_pos_x + marquee_text_.length() + dx_ > display_window_width) {
        dx_ = -dx_;
        current_pos_x += dx_;
    }
    else {
        current_pos_x += dx_;
    }

    if (current_pos_y + dy_ < min_marquee_pos_y_ || current_pos_y + dy_ > display_window_height - bounds_bottom_pad_) {
        dy_ = -dy_;
        current_pos_y += dy_;
    }
    else {
        current_pos_y += dy_;
    }
}

void MarqueeWorkerThread::PrintHeaderLayout() {
    ConsoleManager console_manager;
    console_manager.WriteAtPosition("****************************************", 0, 0);
    console_manager.WriteAtPosition("* Displaying a marquee console! *", 0, 1);
    console_manager.WriteAtPosition("****************************************", 0, 2);
}

void MarqueeWorkerThread::Run() {
    ConsoleManager console_manager;
    int pos_x = min_marquee_pos_x_, pos_y = min_marquee_pos_y_;

    while (running_) {
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            console_manager.ClearConsole();
            PrintHeaderLayout();
            CursorIncrementText(marquee_text_, pos_x, pos_y);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_rate_));
    }
}
