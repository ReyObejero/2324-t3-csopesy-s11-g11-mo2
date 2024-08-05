#pragma once

#include <string>

class Config {
public:
	struct ConfigParameters {
		int num_cpu;
		std::string scheduler;
		int quantum_cycles;
		int preemptive;
		double batch_process_freq;
		int min_ins;
		int max_ins;
		double delay_per_exec;
		int max_overall_mem;
		int min_mem_per_proc;
		int max_mem_per_proc;
	};

	static void Initialize();
	static ConfigParameters GetConfigParameters();

private:
	static ConfigParameters config_parameters_;

};