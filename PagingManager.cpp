#include "PagingManager.h"
#include <mutex>
#include <random>
#include "Config.h"

PagingManager::PagingManager(int totalMemory) {
	std::random_device rd;
	std::mt19937 gen(rd());

	//Config::GetConfigParameters().min_mem_per_proc = 16;
	//Config::GetConfigParameters().max_mem_per_proc = 16;

	//int minLog = std::ceil(std::log2(Config::GetConfigParameters().min_mem_per_proc));
	//int maxLog = std::floor(std::log2(Config::GetConfigParameters().max_mem_per_proc));

	//Memory Section
	int minLog = std::ceil(std::log2(32));
	int maxLog = std::floor(std::log2(32));
	std::uniform_int_distribution<> dist(minLog, maxLog);
	int randomExponent = dist(gen);
	int requiredMemory = 1 << randomExponent;
	this->memProc = requiredMemory;

	//minLog = std::ceil(std::log2(Config::GetConfigParameters().min_page_per_proc));
	//maxLog = std::floor(std::log2(Config::GetConfigParameters().max_page_per_proc))
	
	//Paging Section;
	minLog = std::ceil(std::log2(16));
	maxLog = std::floor(std::log2(16));
	std::uniform_int_distribution<> rng(minLog, maxLog);
	randomExponent = rng(gen);
	int pagePerProcess = 1 << randomExponent;

	this->pageSize = ceil(static_cast<double>(this->memProc) / static_cast<double>(pagePerProcess));
	this->maxFrames = totalMemory / this->pageSize;
	this->currentFrames = this->maxFrames;

	this->expectedFramesPerProcess = ceil(static_cast<double>(this->memProc) / static_cast<double>(pageSize));
}

bool PagingManager::isAllocated(int ret) {
	return ret != -999;
}


int PagingManager::allocateMemory(Process* process) {
	std::lock_guard<std::mutex> lock(this->memoryMutex);
	int remainder = this->currentFrames - process->frames_to_allocate;

	if (this->currentFrames == 0)
		return -999;

	if (remainder >= 0) {
		this->currentFrames = this->currentFrames - process->frames_to_allocate;
		process->frames_to_allocate = 0;
		return 1;
	}

	else {
		process->frames_to_allocate = process->frames_to_allocate - this->currentFrames;
		this->currentFrames = 0;
		return -999;
	}
}

void PagingManager::deallocateMemory(Process* process) {
	std::lock_guard<std::mutex> lock(this->memoryMutex);
	int back = this->memProc / this->pageSize - process->frames_to_allocate;
	process->frames_to_allocate = this->memProc / this->pageSize;
	this->currentFrames += back;
}

void PagingManager::snapshot(int cycle) {

}