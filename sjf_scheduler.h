#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "process.h"
#include <list>

// Custom comparator for priority queue to sort processes by remaining commands
struct CompareProcess {
    bool operator()(Process* const& p1, Process* const& p2) {
        return p1->total_commands > p2->total_commands;
    }
};

class SJF_Scheduler {
private:
    int num_cores;
    bool running;
    std::vector<std::thread> cpu_threads;
    std::priority_queue<Process*, std::vector<Process*>, CompareProcess> process_queue;
    std::list<Process*> running_processes;
    std::vector<Process*> finished_processes;
    std::mutex mtx;
    std::condition_variable cv;

public:
    SJF_Scheduler(int cores);
    ~SJF_Scheduler();

    void add_process(Process* proc);
    void start();
    void stop();
    void cpu_worker(int core_id);
    void print_running_processes();
    void print_finished_processes();
    void screen_ls();
    void print_process_details(const std::string& process_name, int screen);
    void print_process_queue_names();
    void SetCpuCore(int cores);
    void ReportUtil();
    bool isValidProcessName(const std::string& process_name);
    void print_CPU_UTIL();
};