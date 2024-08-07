#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "Process.h"
#include "MemoryManager.h"

class FirstFitManager : public MemoryManager {
public:
    FirstFitManager(int totalMemory);
    int allocateMemory(Process* process) override;
    void deallocateMemory(Process* process) override;
    bool isAllocated(int ret) override;
    
    std::vector<int>& getMemoryBlockList();

    void snapshot(int quantumCycle) override;

private:
    std::unique_ptr<std::vector<int>> memory;

};

