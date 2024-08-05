#include <string>
#include <atomic>
#include <thread>

class MarqueeWorkerThread {
public:
    MarqueeWorkerThread(int refresh_rate, std::string marquee_text);
    void SetBoundsBottomPad(int pad);
    void Start();
    void Stop();

private:
    void Run();
    void PrintHeaderLayout();
    void CursorIncrementText(const std::string& text, int& current_pos_x, int& current_pos_y);
    bool IsInBounds(int text_length, int pos_x, int pos_y, int display_window_width, int display_window_height);
    int GetBoundsBottomPad();

    int refresh_rate_;
    std::string marquee_text_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    int bounds_bottom_pad_;
    int min_marquee_pos_x_;
    int min_marquee_pos_y_;
    int dx_;
    int dy_;
};