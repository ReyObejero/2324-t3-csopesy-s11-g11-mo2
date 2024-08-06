#include "FirstFitManager.h"
#include <mutex>
#include <random>
#include "Config.h"

FirstFitManager::FirstFitManager(int totalMemory) : memory(std::make_unique<std::vector<int>>(totalMemory, -1)){
	std::random_device rd;
	std::mt19937 gen(rd());

	int minLog = std::ceil(std::log2(Config::GetConfigParameters().min_mem_per_proc));
	int maxLog = std::floor(std::log2(Config::GetConfigParameters().max_mem_per_proc));
	std::uniform_int_distribution<> dist(minLog, maxLog);
	int randomExponent = dist(gen);
	int requiredMemory = 1 << randomExponent;
	this->memProc = requiredMemory;

}

bool FirstFitManager::isAllocated(int ret) {
	return ret != -999;
}

std::vector<int>& FirstFitManager::getMemoryBlockList() {
	return *(this->memory);
}

int FirstFitManager::allocateMemory(Process* process) {
	std::lock_guard<std::mutex> lock(this->memoryMutex);
	
	int counter = 0;
	int addressStart = -1;

	for (int i = 0; i < this->getMemoryBlockList().size(); i++) {
		//std::cout << "Current Addr Pointer = " + std::to_string(i);
		if (this->getMemoryBlockList()[i] == -1) {
			//std::cout << "   = Excellent" << std::endl;
			counter++;

			if (addressStart == -1)
				addressStart = i;

			//If finished.
			if (counter == this->memProc) {
				break;
			}
		}
		else {		
			//std::cout << "   = Fallen" << std::endl;
			counter = 0;
			addressStart = -1;
		}
	}

	//If finished and addressStart is not -1.
	if (counter == this->memProc && addressStart != -1) {
		
		process->startAddress = addressStart;
		process->endAddress = addressStart + this->memProc - 1;

		//Allocates the memory blocks.
		for (int i = 0; i < this->memProc; i++) {
			//std::cout << std::to_string(addressStart + i) << std::endl;
			this->getMemoryBlockList()[addressStart + i] = 1;
		}

		return 1;
	}

	return -999;

}

void FirstFitManager::deallocateMemory(Process* process) {
	if (process->startAddress != -1) {
		for (int i = process->startAddress; i < process->endAddress + 1; i++) {
			this->getMemoryBlockList()[i] = -1;
		}
		process->startAddress = -1;
		process->endAddress = -1;
	}
}

void FirstFitManager :: snapshot(int cycle) {

}




