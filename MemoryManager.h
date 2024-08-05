#pragma once
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "Process.h"

class MemoryManager {
public:
    MemoryManager(int totalMemory);
    bool allocateMemory(Process* process);
    void deallocateMemory(Process* process);
    void snapshot(int quantumCycle);
    void SetTotalMemory(int totalMemory);

private:
    int totalMemory; // max-overall-mem
    int usedMemory;
    std::vector<Process*> memory;
    int calculateFragmentation();

    std::string getCurrentTimestamp();
    void printMemory();
};

#endif // MEMORYMANAGER_H
