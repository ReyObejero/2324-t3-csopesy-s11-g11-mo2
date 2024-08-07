#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

class Process {
private:
    static int next_process_id;

public:
    int frames_to_allocate;
    bool isBackingStored;

    std::string name;
    int total_commands;
    int executed_commands;
    std::ofstream log_file;
    std::chrono::time_point<std::chrono::system_clock> start_time; // start time
    int core_id; // which core this process is assigned
    int process_id; // process id 
    int startAddress = -1; // memory start address
    int endAddress = -1; // memory end address

    Process(const std::string& pname, int commands);
    ~Process();
    std::string get_start_time() const;
    std::string get_status() const;

    void displayProcessInfo() const;
};