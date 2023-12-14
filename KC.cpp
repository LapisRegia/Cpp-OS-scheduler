#include <iostream> 
#include <algorithm>
//#include <iomanip>	//do we even need this?
#include <vector>
//#include <thread>	//uncomment to turn off limiters
#include <fstream>
#include "offset.h"

#define MAXPROCLEN 22

//in hindsight its probably more sane to make it into a 2d vector of ints, coz yknow its easier to the eyes and infitessemally easier to understand

class Job
{
public:
	int process_number;
	int arrival_time;
	int complete_time;
	int turnaround_time;
	int wait_time;
	int burst_time;
	int interrupt_counter;
	int type;
	/*
		0 = job
		1 = cpu
		2 = io
	*/
	int state;
	/*
		primarily used by pre-emptive sjn & round robin.
		0 = non arrivabene 
		1 = ready / in queue
		2 = done
		3 = displayed
	*/
	
	Job() {state = 0;}
	
	int& operator[] (std::string var)
	{
		if (var == "pn")
			return process_number;
		if (var == "st")
			return state;
		if (var == "at")
			return arrival_time;
		if (var == "ct")
			return complete_time;
		if (var == "tt")
			return turnaround_time;
		if (var == "wt")
			return wait_time;
		if (var == "bt")
			return burst_time;
		if (var == "ic")
			return interrupt_counter;
		if (var == "tp")
			return type;
	}
};

std::vector<std::vector<Job>> read_file() // done
{
	std::ifstream file;
	file.open("Joblist.csv");
	
	if(file.is_open())
		std::cout << "is open";
	else
		std::cerr << "Can't open file" << std::endl;
		
	int job_number = 1, arrival_and_burst, proc_id, total_burst;
	std::string job_Name, proc_name;
	std::vector<std::vector<Job>> job_in;
	Job process_head;
	
		//read job details
	while (getline(file, proc_name, ','))
	{
		std::vector<Job> process_list(MAXPROCLEN);
		total_burst = 0;
		process_head = process_list.front();
		process_head["pn"] = job_number;
		
		file >> arrival_and_burst;
		process_head["tp"] = 0;
		process_head["at"] = arrival_and_burst;
		getline(file, proc_name);//skip the '\n' char
		
		// iterate from start to end of vector row, skipping the first element.
		for (Job& process : offset(process_list, 1))
		{
			getline(file, proc_name, ',');
				// add process type to vector elements
			if	(proc_name == "CPU")	proc_id = 1;
			else						proc_id = 2;
			process["tp"] = proc_id;
				// add process burst to vector elements
			file >> arrival_and_burst;
				// add burst time to process head and process burst 
			total_burst += arrival_and_burst;
			process["bt"] = arrival_and_burst;
			
			getline(file, proc_name);//skip the '\n' char
		}
		process_head["bt"] = total_burst;
		
		getline(file, proc_name);//skip the '\n' char
		process_list.front() = process_head;
		
		job_in.push_back(process_list);
		job_number++;
	}
	
	return job_in;
}

int display(std::vector<std::vector<Job>>* job_list) //redundant?
{
	std::vector<Job> job_row;
	Job process_head;
	
	for (std::vector<Job>& job_column : *job_list)
	{
		job_row = job_column;
		process_head = job_row.front();
		std::cout << std::endl << "JOB " << process_head["pn"] << " arrival: " << process_head["at"] << " Burst time: " << process_head["bt"]
				  << " Interrupts: "<< process_head["ic"] << " Completion Time: " << process_head["tt"] << " tunaround: " << process_head["tt"] << " Wait: " << process_head["wt"];
	}
}

int in_cpu(int timer, std::vector<Job>* job_row, Job* job_portion) // done
{
	Job process_head = *job_portion;
	bool was_io_before = false, was_cpu_before = false;
	
	for (Job& process : offset(*job_row, 1))
	{
		/*context switching. using a truth table:
		(CPU = true) (I/O = false)			how to read:
		Before		Current		result
		 /			 /		 	 X			Was CPU before, current is CPU, no con switch
		 /			 X			 / 			Was CPU before, current is I/O, con switch
		 X			 /	 		 /			Not I/O before, current is CPU, con switch
		 X			 X			 X			Unreachable
		if the result is true, increment by one. logic is a XORgate*/
		//case 2
		if (process["tp"] == 2 && was_cpu_before == true)
		{
			timer++;
			process_head["ic"] += 1;
			was_cpu_before = false;
		}
		//case 3
		if (process["tp"] == 1 && was_io_before == true)
		{
			timer++;
			process_head["ic"] += 1;
			was_io_before = false;
		}
			
		process["at"] = timer;								//set arrival time to timer
		process["wt"] -= process["bt"];						//set -burst time to wait time
		while (process["bt"] != 0)
		{
			process["bt"] -= 1;
			timer++;
		}
		process["ct"] = timer;								//set completion time to timer
		process["tt"] = (process["ct"] - process["at"]);	//set turnaround time of process
		process["wt"] += process["tt"];						//set wait time of process
		
		if (process["tp"] == 1)
			was_cpu_before = true;
		else
			was_io_before = true;
		}
	*job_portion = process_head;
	return timer;
}

void fcfs(std::vector<std::vector<Job>> job_list)//pretty much done
{
	std::vector<std::vector<Job>> job_in = job_list;
	std::vector<Job> job_row;
	Job process_head;
	int timer = 1;	
		//translation needed
	for (std::vector<Job>& job_column : job_in )
	{
		job_row = job_column;
		process_head = job_row.front();
		process_head["wt"] -= process_head["bt"];
		
		timer = in_cpu(timer, &job_row, &process_head);

		process_head["ct"] = timer;
		process_head["tt"] = (process_head["ct"] - process_head["at"]);
		process_head["wt"] += process_head["tt"];
		
		std::cout << std::endl << "JOB " << process_head["pn"] << " arrival: " << process_head["at"] << " Burst time: " << process_head["bt"]
				  << " Interrupts: "<< process_head["ic"] << " Completion Time: " << process_head["tt"] << " tunaround: " << process_head["tt"] << " Wait: " << process_head["wt"];
				  
		job_row.front() = process_head;
	}
}

void sjn(std::vector<std::vector<Job>> job_list)	//done
{
	/*
	the original idea was:
	sort() from lowest to high
	then call fcfs and pass the sorted vector.
	turns out it was harder than expected, as sort takes iterators as parameters, and containers are not conventionally sortable.
	
	it seems possible to use template classes & declval decltype expression shenadigans (to use std::sort() but i refuse to think
	to save time and organic compute power use a job table thing
    
    std::sort(s.begin(), s.end(), [](int a, int b) {
        return a > b;
    });
	*/
	std::cout << std::endl <<std::endl << "Shortest Job Next (Non Preemptive version)";
	
	std::vector<std::vector<Job>> job_in = job_list;
	std::vector<Job> job_row;
	std::vector<int> job_table;
	Job process_head;
	int job_table_burst, timer = 1;
	
	for (std::vector<Job>& job_column : job_in )
	{
		job_row = job_column;
		process_head = job_row.front();
		job_table_burst = process_head["bt"];
		job_table.push_back(job_table_burst);
	}
		//while sort() gives a O(nlogn) complexity...
		//sort in decending order
	std::sort(job_table.begin(),job_table.end(), [](int a, int b){
		return a < b;
	});
		//iterating a 2d vector is absurdly slow, giving a O(n^2) complexity
	for (int& table : job_table)
	{
		for (std::vector<Job>& job_column : job_in )
		{
			job_row = job_column;
			process_head = job_row.front();
			if(process_head["bt"] == table)
			{
				process_head["wt"] -= process_head["bt"];
				timer = in_cpu(timer, &job_row, &process_head);
				
				process_head["ct"] = timer;
				process_head["tt"] = (process_head["ct"] - process_head["at"]);
				process_head["wt"] += process_head["tt"];
				if (process_head["wt"] < 0) process_head["wt"] = 0;
				job_row.front() = process_head;
				
				std::cout << std::endl << "JOB " << process_head["pn"] << " arrival: " << process_head["at"] << " Burst time: " << process_head["bt"]
					  << " Interrupts: "<< process_head["ic"] << " Completion Time: " << process_head["tt"] << " tunaround: " << process_head["tt"] << " Wait: " << process_head["wt"];
				//job_table.erase(); to reduce time traversing arrived and finished jobs, erase from the vector.
			}
		}
	}
}

void sjf(std::vector<std::vector<Job>> job_list)
{
	/*
		the most confusing of all
	
		every process starts with a state 0 = not arrived.
		every cpu cycle, check if any of the jobs are arrived or not (requires modification of in_cpu function)
		using the job vectors and a separate external table. at a certain cycle where (job arrival time == cpu cycle),
		check whether if the job burst is lower than current
		
		has 2 cycles
		fetch cycle - grabs and stores the next one in the list, prioritizing it if it meets the criteria
		cpu cycle	- self explanatory

		std::vector<std::vector<int>> job_table_2d;
			job	arrival	burst	status
			1	1		919		0	
			2	2		927		0	
			3	37		984		1
	*/
	std::cout << std::endl <<std::endl << "Shortest Job Next (Preemptive version)";
	std::vector<std::vector<Job>> job_in = job_list;
	std::vector<Job> job_row;
	Job process_head;
	std::vector<std::vector<int>> job_table_2d;
	
	int job_table_burst, job_count, timer = 1, current_job, prioritized_job = 1;
	
	for (std::vector<Job>& job_column : job_in )
	{
		job_row = job_column;
		Job process_head = job_row.front();
		std::vector<int> job_table_row {process_head["pn"], process_head["at"], process_head["bt"], process_head["st"]};
		job_table_2d.push_back(job_table_row);
	}
	job_count = job_table_2d.size();
	
	bool was_interrupted = false, interrupt_called = false, was_io_before = false, was_cpu_before = false;
	
		// forgive me, this is one of the ways to save processing time, and i personally dislike inefficient code. i was born in the lag, and i was made man by the lag
	somewhere:
	if (interrupt_called)
	{
		interrupt_called = false;
		was_interrupted = true;
	}
	
	for (std::vector<Job>& job_column : offset( job_in, prioritized_job - 1))
	{
		job_row = job_column;
		Job process_head = job_row.front();
		
		current_job = process_head["pn"];
		
		for (Job& process : offset(job_row, 1))
		{
			//while no interrupt called, while process burst is more than 0
			while( process["st"] != 3 && process["bt"] != 0 && !interrupt_called)
			{
				process["st"] = 2;
				process["at"] = timer;
				// jobfetch();
				for (int i = 0; i < 3; i++)
				{
					//fetch the incoming job
					if(job_table_2d[i][1] == timer)
					{
						//compare current job to burst time of incoming job
						if(job_table_2d[i][2] < process_head["bt"]) 
						{
							process_head["ic"] ++;
							job_table_2d[current_job][3] = 1;
							prioritized_job = job_table_2d[i][0];
							std::cout << std::endl << "Job Pre-emptively Interrupted. Current Job OUT: Job " << process_head["pn"] << " Job IN: Job " << prioritized_job;
							timer++;
							interrupt_called = true;
							job_row.front() = process_head;
							goto somewhere;
						}
					}
				}
				if (process["tp"] == 2 && was_cpu_before == true)
				{
					timer++;
					process_head["ic"] += 1;
					was_cpu_before = false;
				}
				if (process["tp"] == 1 && was_io_before == true)
				{
					timer++;
					process_head["ic"] += 1;
					was_io_before = false;
				}
				if (!interrupt_called)
				{
					timer++;
					process["bt"] -= 1;
					process_head["wt"] -= 1;
				}
				if (process["tp"] == 1)
					was_cpu_before = true;
				else
					was_io_before = true;
			}
			process["st"] = 3;
		}
		process_head["st"] =  2;
		//after processing all processes, set job status to done (2)
		//std::cout << 	process_head["st"];
		
		//if the job is labelled done - save and calculate data, then display.
		if (process_head["st"] ==  2 )
		{
			process_head["ct"] = timer;
			process_head["tt"] = (process_head["ct"] - process_head["at"]);
			process_head["wt"] += process_head["tt"];
			process_head["st"] = 3;
			
			job_row.front() = process_head;
			
			std::cout << std::endl <<"JOB " << process_head["pn"] << " arrival: " << process_head["at"] << " Burst time: " << process_head["bt"]
					  << " Interrupts: "<< process_head["ic"] << " Completion Time: " << process_head["tt"] << " tunaround: " << process_head["tt"] << " Wait: " << process_head["wt"];
		}
		
		if(was_interrupted)
		{
			job_in.erase(job_in.begin() + (prioritized_job-1));
			prioritized_job = 1;
			was_interrupted = false;
			goto somewhere;
		}
	}
}

void roundR(std::vector<std::vector<Job>> job_list)
{
	std::vector<std::vector<Job>> job_in = job_list;
	std::vector<Job> job_row;
	Job process_head;
	int timer = 1, job_count = 0, quantum = 0, current_job;
	/*
	burst time, arrival negligible.
			std::vector<std::vector<int>> job_table_2d;
			job	processses done
			1	19
			2	18
			3	18
	once processes done reaches 20, display and erase the job
	*/
	std::vector<std::vector<int>> job_table_2d;
	for (std::vector<Job>& job_column : job_in )
	{
		job_row = job_column;
		process_head = job_row.front();
		std::vector<int> job_table_row {process_head["pn"], 0};
		job_table_2d.push_back(job_table_row);
	}
	job_count = job_table_2d.size();
	
	bool interrupt_called = false, was_io_before = false, was_cpu_before = false;
	
	while (job_table_2d.size() != 0)
	{
		for (std::vector<Job>& job_column : job_in )
		{
			job_row = job_column;
			process_head = job_row.front();
			
			current_job = process_head["pn"];
			std::cout << "\n in job " << current_job;
			for (int i = 0; i < 3; i++)
			{
				if (job_table_2d[i][0] == current_job)
				{
					job_table_2d[i][1] = 0;
					break;
				}
			}
			quantum = 0;
			for (Job& process : offset(job_row, 1))
			{
				while (quantum != 10 && process["st"] != 3 && process["bt"] != 0)
				{
					std::cout << "\n in quantum. " << quantum;
					process["st"] = 2;
					process["at"] = timer;
					
					if (process["tp"] == 2 && was_cpu_before == true)
					{
						timer++;
						process_head["ic"] += 1;
						was_cpu_before = false;
					}
					if (process["tp"] == 1 && was_io_before == true)
					{
						timer++;
						process_head["ic"] += 1;
						was_io_before = false;
					}
					
					quantum++;
					timer++;
					process["bt"] -= 1;
					process_head["wt"] -= 1;
					
					if (process["tp"] == 1)
						was_cpu_before = true;
					else
						was_io_before = true;
				}
				if (process["bt"] == 0)
				{
					process["st"] = 3;
					for (int i = 0; i < 3; i++)
					{
						if (job_table_2d[i][0] == current_job)
							job_table_2d[i][1] += 1;
					}
				}
				std::cout << "\n in process. burst: " << process["bt"];
				job_row.front() = process_head;
			}
			for (int i = 0; i < 3; i++)
			{
				if (job_table_2d[i][0] == current_job)
				{
					if ( job_table_2d[i][1] == 20)
					{
						job_count -= 1;
						process_head["ct"] = timer;
						process_head["tt"] = (process_head["ct"] - process_head["at"]);
						process_head["wt"] += process_head["tt"];
						process_head["st"] = 3;
						
						job_row.front() = process_head;
						
						std::cout << std::endl <<"JOB " << process_head["pn"] << " arrival: " << process_head["at"] << " Burst time: " << process_head["bt"]
								  << " Interrupts: "<< process_head["ic"] << " Completion Time: " << process_head["tt"] << " tunaround: " << process_head["tt"] << " Wait: " << process_head["wt"];
						job_in.erase(job_in.begin() + (current_job - 1));
						job_table_2d.erase(job_table_2d.begin() + (current_job - 1));
						std::sort(job_table_2d[0].begin(),job_table_2d[0].end(), [](int a, int b){
							return a < b;
						});
						job_table_2d.pop_back();
					//	break;
					}
				}
			}
		}
	}
	
}
int main()
{	
	std::vector<std::vector<Job>> jobvector;
	Job process_head;
	jobvector =	read_file();
	
	std::cout << std::endl;
	fcfs(jobvector);
	sjn(jobvector);
	sjf(jobvector);
	//roundR(jobvector);
	
	return 0;
}
