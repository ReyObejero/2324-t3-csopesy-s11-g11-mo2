#include "MemoryManager.h"
#include <sstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "Config.h"
#include <fstream>
#include <random>
#include <iostream>

MemoryManager::MemoryManager(int totalMemory) : totalMemory(totalMemory), usedMemory(0) {}

bool MemoryManager::allocateMemory(Process* process) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(Config::GetConfigParameters().min_mem_per_proc, Config::GetConfigParameters().max_mem_per_proc);
    int requiredMemory = dist(gen);

    int alignedMemory = 1;
    while (alignedMemory < requiredMemory) {
        alignedMemory *= 2;
    }

    for (int i = 0; i < totalMemory; i += alignedMemory) {
        int alignedStartAddress = i;

        if (alignedStartAddress % alignedMemory != 0) {
            alignedStartAddress = (alignedStartAddress / alignedMemory) * alignedMemory + alignedMemory;
        }

        int alignedEndAddress = alignedStartAddress + alignedMemory;

        bool spaceAvailable = true;
        for (auto& p : memory) {
            if ((alignedStartAddress >= p->startAddress && alignedStartAddress < p->endAddress) ||
                (alignedEndAddress > p->startAddress && alignedEndAddress <= p->endAddress)) {
                spaceAvailable = false;
                break;
            }
        }

        if (spaceAvailable && alignedEndAddress <= totalMemory) {
            process->startAddress = alignedStartAddress;
            process->endAddress = alignedEndAddress;
            memory.push_back(process);
            usedMemory += alignedMemory;
            return true;
        }
    }
    return false;
}



void MemoryManager::deallocateMemory(Process* process) {
    auto it = std::remove(memory.begin(), memory.end(), process);
    if (it != memory.end()) {
        usedMemory -= (process->endAddress - process->startAddress);
        memory.erase(it);
    }
}

void MemoryManager::snapshot(int quantumCycle) {
    std::string timestamp = getCurrentTimestamp();
    int fragmentation = calculateFragmentation();

    std::ofstream file("memory_stamp_" + std::to_string(quantumCycle) + ".txt");
    file << "Timestamp: " << timestamp << "\n";
    file << "Number of processes in memory: " << memory.size() << "\n";
    file << "Total external fragmentation in KB: " << fragmentation << "\n\n";

    file << "----end---- = " << totalMemory << "\n";
    file << std::endl;

    // Create a copy of memory
    std::vector<Process*> memoryCopy(memory);

    // Sort memoryCopy by endAddress (greatest to lowest)
    std::sort(memoryCopy.begin(), memoryCopy.end(), [](Process* a, Process* b) {
        return a->endAddress > b->endAddress;
        });

    for (auto& process : memoryCopy) {
        file << process->endAddress << "\n";
        file << process->name << "\n";
        file << process->startAddress << "\n\n";
    }

    file << "----start---- = " << 0 << "\n";

    file.close();
}

void MemoryManager::SetTotalMemory(int totalMemory) {
    this->totalMemory = totalMemory;
}

int MemoryManager::calculateFragmentation() {
    int fragmentation = totalMemory - usedMemory;
    // return fragmentation / 1024; // in KB
    return fragmentation;
}

std::string MemoryManager::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    std::tm tm_now;
    localtime_s(&tm_now, &now);
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%d/%m/%Y %I:%M:%S%p");
    return oss.str();
}

void MemoryManager::printMemory() {
    for (auto& process : memory) {
        std::cout << process->endAddress << "\n";
        std::cout << process->name << "\n";
        std::cout << process->startAddress << "\n";
    }
    std::cout << "----end---- = " << totalMemory << "\n";
}
