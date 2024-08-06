#include <mutex>
#include "Process.h"
#include "MemoryManager.h"

class PagingManager : public MemoryManager {
public:
    PagingManager(int totalMemory);

    int allocateMemory(Process* process);
    bool isAllocated(int ret);
    void deallocateMemory(Process* process);
    void snapshot(int cycle);

    int maxFrames;
    int currentFrames;
    int memProc;
    int pageSize;
    int expectedFramesPerProcess;
    std::mutex memoryMutex;
};
