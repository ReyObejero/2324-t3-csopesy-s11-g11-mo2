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
#include <mutex>

class MemoryManager {
public:
    std::mutex memoryMutex;
    std::mutex fileMutex;
    int memProc; // randomized mem-proc

    int expectedFramesPerProcess;

    virtual int allocateMemory(Process* process) = 0;
    virtual void deallocateMemory(Process* process) = 0;
    virtual void snapshot(int quantumCycle) = 0;

    int getTotalMemory();
    //virtual void SetTotalMemory(int totalMemory);
    virtual bool isAllocated(int ret) = 0;

    static struct process_data {
        int process_id;
        std::string name;
        int executed_commands;
        int pages;
    };

    void store_process(Process* process);
    process_data load_process(Process* process);

private:
    int totalMemory; // max-overall-mem

    int usedMemory;
    std::vector<Process*> memory;
    int calculateFragmentation();
    std::string getCurrentTimestamp();
    void printMemory();
};

static int type;


#endif // MEMORYMANAGER_H