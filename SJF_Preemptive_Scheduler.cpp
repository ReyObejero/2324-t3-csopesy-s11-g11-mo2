#include "SJF_Preemptive_Scheduler.h"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>

SJF_Preemptive_Scheduler::SJF_Preemptive_Scheduler(int cores)
    : num_cores(cores), running(true) {}

SJF_Preemptive_Scheduler::~SJF_Preemptive_Scheduler() {
    stop(); // Ensure all threads are stopped and resources cleaned up
}

void SJF_Preemptive_Scheduler::add_process(Process* proc) {
    std::lock_guard<std::mutex> lock(mtx);
    ready_queue.push(proc);
    cv.notify_one();
}

void SJF_Preemptive_Scheduler::start() {
    for (int i = 0; i < num_cores; ++i) {
        cpu_threads.emplace_back(&SJF_Preemptive_Scheduler::cpu_worker, this, i);
    }
    std::cout << "Scheduler started with " << num_cores << " cores.\n";
}

void SJF_Preemptive_Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
    }
    cv.notify_all();
    for (auto& thread : cpu_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "Scheduler stopped.\n";
}

void SJF_Preemptive_Scheduler::cpu_worker(int core_id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> exec_dist(1, 10);

    while (running) {
        Process* proc = nullptr;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return !ready_queue.empty() || !running; });

            if (!running && ready_queue.empty()) break;

            proc = ready_queue.top();
            ready_queue.pop();
            proc->core_id = core_id;
            proc->start_time = std::chrono::system_clock::now();
            running_processes.push_back(proc); // Add process to running_processes
        }

        while (proc->executed_commands < proc->total_commands) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                proc->executed_commands += exec_dist(gen); // Execute for a random time slice
              
                if (proc->executed_commands > proc->total_commands) {
                    proc->executed_commands = proc->total_commands; // Ensure we don't exceed total_commands
                }
            }

            // Sleep for a defined duration (e.g., 250 milliseconds)
            std::this_thread::sleep_for(std::chrono::milliseconds(250));

            // Reinsert the process into ready queue if it still has commands to execute and a shorter process arrives
            if (!ready_queue.empty() && ready_queue.top()->total_commands - ready_queue.top()->executed_commands < proc->total_commands - proc->executed_commands) {
                std::lock_guard<std::mutex> lock(mtx);
                ready_queue.push(proc);
                running_processes.remove(proc); // Remove from running_processes
                break; // Break out of execution loop to allow scheduling of other processes
            }
        }

        // If process has finished all commands, move it to finished queue
        if (proc->executed_commands >= proc->total_commands) {
            std::lock_guard<std::mutex> lock(mtx);
            finished_processes.push_back(proc);
            running_processes.remove(proc); // Remove from running_processes
        }
    }
}


void SJF_Preemptive_Scheduler::print_running_processes() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Running processes:\n";
    for (auto& proc : running_processes) {
        std::cout << proc->name << " (Core: " << proc->core_id << ") " << proc->get_status() << "\n";
    }
    std::cout << "----------------\n";
}



void SJF_Preemptive_Scheduler::print_finished_processes() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Finished processes:\n";
    for (auto& proc : finished_processes) {
        std::cout << proc->name << " (" << proc->get_start_time() << ") Finished "
            << proc->executed_commands << " / " << proc->total_commands << "\n";
    }
    std::cout << "----------------\n";
}

void SJF_Preemptive_Scheduler::screen_ls() {
    print_CPU_UTIL();
    print_running_processes();
    print_finished_processes();
}

void SJF_Preemptive_Scheduler::print_CPU_UTIL() {
    int numOfRunningProcess = 0;
    int numOfFinishedProcess = 0;
    int cpuUtilization = 0;
    for (auto& proc : running_processes) {
        numOfRunningProcess++;
    }
    for (auto& proc : finished_processes) {
        numOfFinishedProcess++;
    }
    if (numOfRunningProcess == num_cores) {
        cpuUtilization = 100;

    }
    else if (numOfRunningProcess == 0) {
        cpuUtilization = 0;
    }
    std::cout << "Cpu Utilization: " << cpuUtilization << "%\n";
    std::cout << "Cores Used: " << numOfRunningProcess << "\n";
    std::cout << "Cores Available: " << num_cores - numOfRunningProcess << "\n";

    std::cout << "----------------\n";
}


void SJF_Preemptive_Scheduler::ReportUtil() {
    int numOfRunningProcess = 0;
    int numOfFinishedProcess = 0;
    int cpuUtilization = 0;
    for (auto& proc : running_processes) {
        numOfRunningProcess++;
    }
    for (auto& proc : finished_processes) {
        numOfFinishedProcess++;
    }
    if (numOfRunningProcess == num_cores) {
        cpuUtilization = 100;

    }
    else if (numOfRunningProcess == 0) {
        cpuUtilization = 0;
    }

    std::vector<int> cores_used;
    int total_executed_commands = 0;
    int total_commands = 0;

    {
        std::lock_guard<std::mutex> lock(mtx);

        for (auto& proc : running_processes) {
            total_executed_commands += proc->executed_commands;
            total_commands += proc->total_commands;

            if (std::count(cores_used.begin(), cores_used.end(), proc->core_id) == 0) {
                cores_used.push_back(proc->core_id);
            }
        }

        for (auto& proc : finished_processes) {
            total_executed_commands += proc->executed_commands;
            total_commands += proc->total_commands;
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream log("csopesy-log.txt", std::ios::app);
    log << "CPU Utilization: " << cpuUtilization << "%" << std::endl;
    log << "Cores Used: " << cores_used.size() << std::endl;
    log << "Cores Available: " << num_cores - cores_used.size() << std::endl;
    log << "----------------\n";
    log << "Running processes:\n";
    for (auto& proc : running_processes) {
        if (std::count(cores_used.begin(), cores_used.end(), proc->core_id) == 0) {
            cores_used.push_back(proc->core_id);
        }

        log << proc->name << " (" << proc->get_start_time() << ") Core: "
            << (proc->core_id == -1 ? "N/A" : std::to_string(proc->core_id))
            << " " << proc->executed_commands << " / " << proc->total_commands << "\n";
    }
    log << std::endl;
    log << "Finished processes:\n";
    for (auto& proc : finished_processes) {
        log << proc->name << " (" << proc->get_start_time() << ") Finished "
            << proc->executed_commands << " / " << proc->total_commands << "\n";
    }
    log << "----------------\n";
    log << std::endl;
    std::cout << "Report generated at /csopesy-log.txt" << std::endl;
}

void SJF_Preemptive_Scheduler::print_process_details(const std::string& process_name, int screen) {
    std::lock_guard<std::mutex> lock(mtx);

    // Check process_queue
    std::priority_queue<Process*, std::vector<Process*>, CompareProcessB> temp_queue = ready_queue;
    while (!temp_queue.empty()) {
        Process* proc = temp_queue.top();
        temp_queue.pop();
        if (proc->name == process_name) {
            proc->displayProcessInfo();
            return;
        }
    }

    // Check running_processes
    for (auto& proc : running_processes) {
        if (proc->name == process_name && screen == 0) {
            system("cls"); // Clear screen for better readability
            proc->displayProcessInfo();
            return;
        }
        else if (proc->name == process_name && screen == 1) {
            proc->displayProcessInfo();
            return;
        }
    }

    // Check finished_processes
    for (auto& proc : finished_processes) {
        if (proc->name == process_name && screen == 1) {
            proc->displayProcessInfo();
            std::cout << "Process " << process_name << " has finished and cannot be accessed after exiting this screen.\n";
            return;
        }
        else if (proc->name == process_name && screen == 0) {
            std::cout << "Process " << process_name << " not found.\n";
            return;
        }
    }

    // If process not found in any list
    std::cout << "Process " << process_name << " not found.\n";
}

void SJF_Preemptive_Scheduler::SetCpuCore(int cores) {
    this->num_cores = cores;
}

bool SJF_Preemptive_Scheduler::isValidProcessName(const std::string& process_name) {
    std::lock_guard<std::mutex> lock(mtx);

    // Check process_queue
    std::priority_queue<Process*, std::vector<Process*>, CompareProcessB> temp_queue = ready_queue;
    while (!temp_queue.empty()) {
        Process* proc = temp_queue.top();
        temp_queue.pop();
        if (proc->name == process_name) {
            return false;
        }
    }

    // Check running_processes
    for (auto& proc : running_processes) {
        if (proc->name == process_name) {
            return false;
        }
    }

    // Check finished_processes
    for (auto& proc : finished_processes) {
        if (proc->name == process_name) {
            return false;
        }
    }

    return true;
}