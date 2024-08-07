#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "Process.h"
#include "MemoryManager.h"

class PagingManager : public MemoryManager {
public:
    int maxFrames;
    int currentFrames;
    int pageSize;


    PagingManager(int totalMemory);
    int allocateMemory(Process* process) override;
    void deallocateMemory(Process* process) override;
    bool isAllocated(int ret) override;
    void snapshot(int quantumCycle) override;

private:


};