#include "RRScheduler.h"
#include <iostream>
#include <chrono>
#include "Process.h"
#include "Config.h"
#include <algorithm>
#include <random>
#include "MemoryManager.h"
#include "PagingManager.h"
#include "FirstFitManager.h"


RR_Scheduler::RR_Scheduler(int cores, int quantum) : num_cores(cores), time_quantum(quantum), running(false) {}

RR_Scheduler::~RR_Scheduler() {
    stop();
}

void RR_Scheduler::add_process(Process* proc) {
    std::lock_guard<std::mutex> lock(mtx);

    if (Config::GetConfigParameters().max_mem_per_proc == 1 && Config::GetConfigParameters().min_page_per_proc == 1) {

    }
    else {
        PagingManager* paging = dynamic_cast<PagingManager*>(memoryManager);
        proc->frames_to_allocate = paging->memProc / paging->pageSize;
    }
    process_queue.push(proc);
    cv.notify_one();
}

void RR_Scheduler::start() {

    if (!memoryManager) {
        if (Config::GetConfigParameters().min_page_per_proc == 1 && Config::GetConfigParameters().max_page_per_proc == 1) {
            memoryManager = new FirstFitManager(Config::GetConfigParameters().max_overall_mem);
        }
        else {
            memoryManager = new PagingManager(Config::GetConfigParameters().max_overall_mem);
        }
    }

    running = true;
    start_time = std::chrono::steady_clock::now(); // Record the start time
    for (int i = 0; i < num_cores; ++i) {
        isStealing = false;
        cpu_threads.emplace_back(&RR_Scheduler::cpu_worker, this, i);
    }
    //std::cout << "Scheduler started with " << num_cores << " cores.\n";
}

void RR_Scheduler::stop() {
    running = false;
    cv.notify_all();
    for (std::thread& t : cpu_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    //std::cout << "Scheduler stopped.\n";
}

void RR_Scheduler::cpu_worker(int core_id) {
    int quantumCycle = 0;

    while (running) {
        Process* proc = nullptr;

        {
            std::unique_lock<std::mutex> lock(this->mtx);

            cv.wait(lock, [&] { return !process_queue.empty() || !running; });

            if (!running && process_queue.empty()) break;

            proc = process_queue.front();
            process_queue.pop();

            if (proc->isBackingStored) {
                //std::cout << proc->process_id << " = From Backing Store" << std::endl;
                memoryManager->load_process(proc);
                //proc->isBackingStored = false;
            }

            //first try
            if (!memoryManager->isAllocated(memoryManager->allocateMemory(proc))) {
                //std::cout << "First attempt failed." << std::endl;
                //second try, deallocate/backingstore random allocated process on process queue
                //third try, deallocate/backingstore the other core's process

                bool isSecondTrySuccess = false;
                std::queue<Process*> copy = process_queue;
                //std::queue<Process*> updated;
                std::vector<Process*> converted;

                while (!copy.empty()) {
                    Process* p = copy.front();
                    copy.pop();
                    if (dynamic_cast<FirstFitManager*>(memoryManager) != nullptr) {
                        if (p->startAddress != -1)
                            converted.push_back(p);
                        else {
                            //updated.push(p);
                        }
                    }
                    else {
                        PagingManager* pagingManager = dynamic_cast<PagingManager*>(memoryManager);
                        if (pagingManager->expectedFramesPerProcess > p->frames_to_allocate) {
                            converted.push_back(p);
                        }
                        else {
                            //updated.push(p);                             
                        }
                    }
                }


                if (converted.size() > 0) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dist(0, converted.size() - 1);
                    shuffle(converted.begin(), converted.end(), rd);

                    for (Process* shuffled : converted) {
                        memoryManager->deallocateMemory(shuffled);
                        memoryManager->store_process(shuffled);
                        isSecondTrySuccess = memoryManager->isAllocated(memoryManager->allocateMemory(proc));
                        if (isSecondTrySuccess) {
                            break;
                        }
                    }
                }

                if (!isSecondTrySuccess) {
                    //std::cout << "Second attempt failed." << std::endl;

                    std::unique_lock<std::mutex> stealLock(this->stealMtx);
                    isStealing = true;

                    stealCv.wait_for(stealLock, std::chrono::seconds(3), [&] { return !isStealing; });
                    bool isThirdTrySuccess = memoryManager->isAllocated(memoryManager->allocateMemory(proc));

                    if (!isThirdTrySuccess) {
                        //std::cout << "Third attempt failed." << std::endl;
                        process_queue.push(proc);
                        continue;
                    }

                    else {

                    }
                }

                else {
                    //Second success

                }
            }

            proc->core_id = core_id;
            proc->start_time = std::chrono::system_clock::now();
            running_processes.push_back(proc);
        }

        int remaining_commands = proc->total_commands - proc->executed_commands;
        int commands_to_execute = std::min(time_quantum, remaining_commands);
        int executed_in_quantum = 0;

        while (commands_to_execute > 0 && proc->executed_commands < proc->total_commands) {
            {
                if (isStealing) {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (isStealing) {
                        isStealing = false;
                        memoryManager->deallocateMemory(proc);
                        memoryManager->store_process(proc);
                        process_queue.push(proc);
                        stealCv.notify_one();
                        proc = nullptr;
                        break;
                    }
                }

                proc->executed_commands += Config::GetConfigParameters().quantum_cycles;
                executed_in_quantum += Config::GetConfigParameters().quantum_cycles;
                if (proc->executed_commands >= proc->total_commands) {
                    proc->executed_commands = proc->total_commands;
                }

                //std::cout << "End Loop Execute..." << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(Config::GetConfigParameters().delay_per_exec * 1000)));

            if (executed_in_quantum >= time_quantum) {

                //std::cout << "Count = " << proc->executed_commands << std::endl;
                break;
            }
        }

        //memoryManager->snapshot(++quantumCycle);

        if (proc) {
            std::lock_guard<std::mutex> lock(mtx);
            if (proc->executed_commands < proc->total_commands) {
                process_queue.push(proc);
                running_processes.remove(proc);
            }
            else {
                //std::cout << "Finished name of process = " << proc->process_id << std::endl;
                running_processes.remove(proc);
                memoryManager->deallocateMemory(proc);
                finished_processes.push_back(proc);

            }
        }
    }
}

void RR_Scheduler::screen_ls() {
    //std::lock_guard<std::mutex> lock(mtx);
    print_CPU_UTIL();
    print_running_processes();
    print_finished_processes();

}

void RR_Scheduler::SetCpuCore(int cpu_core) {
    this->num_cores = cpu_core;
}

void RR_Scheduler::SetQuantum(int quantum) {
    this->time_quantum = quantum;
}

bool RR_Scheduler::isValidProcessName(const std::string& process_name)
{
    std::lock_guard<std::mutex> lock(mtx);

    // Check process_queue
    std::queue<Process*> temp_queue = process_queue;
    while (!temp_queue.empty()) {
        Process* proc = temp_queue.front();
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

void RR_Scheduler::ReportUtil() {
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


void RR_Scheduler::print_running_processes() {
    std::cout << "Running processes:\n";
    for (auto& proc : running_processes) {
        std::cout << proc->name << "\t(" << proc->get_start_time() << ")\t\tCore: "
            << (proc->core_id == -1 ? "N/A" : std::to_string(proc->core_id))
            << "\t\t" << proc->executed_commands << " / " << proc->total_commands << "\n";

    }
    std::cout << std::endl;
}
void RR_Scheduler::print_finished_processes() {

    std::cout << "Finished processes:\n";
    for (auto& proc : finished_processes) {
        std::cout << proc->name << "\t(" << proc->get_start_time() << ")" << "\t\tFinished\t"
            << proc->executed_commands << " / " << proc->total_commands << "\n";
    }
    std::cout << "--------------------------------------------------\n";
    std::cout << std::endl;
}
void RR_Scheduler::print_CPU_UTIL() {
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
    std::cout << std::endl;
    std::cout << "--------------------------------------------------\n";
}



void RR_Scheduler::print_process_details(const std::string& process_name, int screen) {
    std::lock_guard<std::mutex> lock(mtx);

    // Check process_queue
    std::queue<Process*> temp_queue = process_queue;
    while (!temp_queue.empty()) {
        Process* proc = temp_queue.front();
        temp_queue.pop();
        if (proc->name == process_name) {
            if (screen == 0) {
                system("cls");
            }

            proc->displayProcessInfo();
            return;
        }
    }

    // Check running_processes
    for (auto& proc : running_processes) {
        if (proc->name == process_name && screen == 0) {
            system("cls");
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

int RR_Scheduler::GetCpuUtilization() {
    std::vector<int> active_cores;

    for (auto& process : this->running_processes) {
        if (!(std::count(active_cores.begin(), active_cores.end(), process->core_id))) {
            active_cores.push_back(process->core_id);
        }
    }

    return (active_cores.size() / static_cast<float>(this->num_cores)) * 100;
}