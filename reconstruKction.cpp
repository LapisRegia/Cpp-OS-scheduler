#include <iostream> 
#include <algorithm>
#include <vector>
#include <thread>	
#include <fstream>
#include <chrono> //do we need this? check if results are constant 

std::vector<std::vector<std::vector<int>>> read_file()
{
	std::ifstream file;
	file.open("Joblist.csv");
		if(file.is_open())
		std::cout << "is open";
	else
		std::cerr << "Can't open file" << std::endl;
	
	int job_number = 1, process_number = 1, process_type, total_burst;
	std::string job_Name, proc_name;
	std::vector<std::vector<std::vector<int>>> job_list;
	int type, arrival, burst, complete, turnaround, wait, interrupt, state;
	
	/*
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	State
	1		Job		1		919		919			0			0		7			0
	*/
	
	while (getline (file, proc_name, ','))
	{
		std::vector<std::vector<int>> job;
		total_burst = 0;
		process_number = 1;
		// get the number
		file >> arrival;
		// skip the '\n' char
		getline(file, proc_name);
		// create the process_head vector and push back into the column
		std::vector<int> process_head {job_number, 0, arrival, 0,0,0,0,0,0};
		job.push_back(process_head);
		
		for (int i = 0; i < 21; i++) // to make it more versatile, put in another getline (file, proc_name, ',')
		{
			// get and add process type to process
			getline(file, proc_name, ',');
			if	(proc_name == "CPU")
				process_type = 1;
			else					
				process_type = 2;
			file >> burst;
			total_burst += burst;
			
			// create the process vector and push back into the column
			std::vector<int> processes {process_number, process_type, 0, burst ,0,0,0,0,0};
			job.push_back(processes);
			
			// skip the '\n' char
			getline(file, proc_name);
			process_number++;
		}
		job[0][3] = total_burst;
		// skip the '\n' char
		getline(file, proc_name);
		job_number++;
		// push back the column into the row of columns
		job_list.push_back(job);
	}
	return job_list;
}

void fcfs (std::vector<std::vector<std::vector<int>>> job_list)
{
	int timer = 1, avg_wait = 0;
	bool was_io_before = false, was_cpu_before = false;
	/*
		0	1		2		3		4				5		6			7		8
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	State
	1		Job		1		919		0			0			0		7			0
	*/
	
	for (int z = 0; z < 3; z++)
	{
		/*set job wait time to (negative) total job burst time
			mathematically makes sense, 
			wait = complete - total burst
			add total burst to wait first, then add complete time
		*/
		job_list[z][0][6] -= job_list[z][0][3];
		
		for(int y = 1; y < 22; y ++)
		{
			/*
				context switching. using a truth table:
				(CPU = true) (I/O = false)			how to read:
				Before		Current		result
				 /			 /		 	 X			Was CPU before, current is CPU, no con switch
				 /			 X			 / 			Was CPU before, current is I/O, con switch (or not)
				 X			 /	 		 /			Not I/O before, current is CPU, con switch
				 X			 X			 X			Unreachable
				if the result is true, increment by one. logic is a XORgate
			*/
			//case 2
			if (job_list[z][y][1] == 2 && was_cpu_before == true)
			{
				timer++;
				job_list[z][0][7]++;
				was_cpu_before = false;
			}
			//case 3
			if (job_list[z][y][1] == 1 && was_io_before == true)
			{
				timer++;
				job_list[z][0][7]++;
				was_io_before = false;
			}
			//while PROCESS burst time !=0, decrement burst time and increment timer
			while (job_list[z][y][3] != 0)
			{
				job_list[z][y][3] -=1;
				timer++;
			}
			if (job_list[z][y][1] == 1)
				was_cpu_before = true;
			else
				was_io_before = true;
		}
		// save complete time to process head
		job_list[z][0][4] = timer;
		// save turnaround time = complete time - arrival
		job_list[z][0][5] = (job_list[z][0][4] - job_list[z][0][2]);
		// save wait time
		job_list[z][0][6] += job_list[z][0][5];
		if ( job_list[z][0][6] < 0)
		 job_list[z][0][6] = 0;
		// add job wait time to avg_wait variable
		avg_wait += job_list[z][0][6];
		
		std::cout << std::endl << "JOB " << job_list[z][0][0] << " arrival: " << job_list[z][0][2] << " Burst time: " << job_list[z][0][3]
				  << " Interrupts: "<< job_list[z][0][7] << " Completion Time: " << job_list[z][0][4] << " tunaround: " << job_list[z][0][5] << " Wait: " << job_list[z][0][6]; 
	}
	std::cout << std::endl << "Average wait time = " << avg_wait / 3 << " cycles";
}

void sjn(std::vector<std::vector<std::vector<int>>> job_list)
{
	std::cout << "\n\nShortest Job Next (Non Preemptive version)";
	/*
		In the assignment guidelines, Jobs are assumed to be in ready queue
		so in that sense, arrival time is negigible (since program SORTS the ready queue first before executing.)
		
		The idea is:
			sort() the jobs from lowest to high
			call fcfs.
	*/
	/*
		uh yeah. for a 1-Dimensional vector, it would look like this
		std::sort(job_table.begin(),job_table.end(), []
		(int a, int b){
			return a < b;
		});
		
		for a 2d v=ector;
		std::sort(job_table.begin(),job_table.end(), []
		(const std::vector<int>& v1, const std::vector<int>& v2){
			return return v1[x] < v2[x];
		});
		where x is the desired column in a 2x2 matrix.
		
		for a 3d vector, is below. the pattern is simple; add another std::vector<>
		 should you wish to sort by process burst time, add another [3] to both lhs and rhs.
		complexity is O(nlogn).
	*/
	std::sort(job_list.begin(),job_list.end(), []
		(const std::vector<std::vector<int>>& lhs, const std::vector<std::vector<int>>& rhs){
		return lhs[0][3] < rhs[0][3];
	});
	
	fcfs(job_list);
}

void sjf (std::vector<std::vector<std::vector<int>>> job_list)
{
	std::cout << "\n\nShortest Job Next (Preemptive version)";
	/*
		the most confusing of all
		dependant on arrival time (or not? since "assume every job is in ready state")
		has 2 cycles, fetch and in_cpu
		fetch runs first, then in_cpu.
		
		uses 2 tables:
		job_list
		burst logger
		burst logger table is necessary to keep track of the current job's remaining burst should a pre emption happen.
			e.g. if incoming job < current job burst
					save current job burst to to burst logger table
				after the prioritized job is finished,
				and when the scheduler starts again to do the unfinished job
				retrieve the remaining job burst from the burst logger table.
		(this is a problem risen from the fact that i'd like to keep the process head in job list constant, so that the job's busrt time can be displayed
		there are more elegant and simple solutions, such as displaying the job details ONCE, and for every algo, every job display ONLY ct, tat, wait and ic.
		however, yeah. my brain can only see so far.)
				 
		in previous versions job 3 would arrive at 200 with a burst time of 827. this will preempt the scheduler
		however it should not make sense, since at 200, job 1's burst time is effectively 719, which is less than job 3.
		with a process logger (the current_burst variable) the scheduler will not pre empt.
		
	*/
	std::vector<std::vector<int>> burst_logger;
	int timer = 1, job_count = 0, prioritized_job = 1 , avg_wait = 0, current_burst = 0;
	bool interrupt_called = false, was_interrupted = false, was_io_before = false, was_cpu_before = false;
	/* for reference
		0	1		2		3		4				5		6			7		8
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	State
	1		Job		1		919		0			0			0		7			0
	*/
	
	for (int job = 0; job < job_list.size(); job++)
	{
		int table_number, table_burst, table_arrival;
		table_number = job_list[job][0][0];
		table_burst = job_list[job][0][2];
		table_arrival = job_list[job][0][1];
		std::vector<int> table_row {table_number, table_arrival, table_burst};
		burst_logger.push_back(table_row);
	}
	
	//	this is the only way to not use up obscene amounts of cpu time
	somewhere:
		
	// was interrupt is used to signal the program to start again one more time AFTER a pre emtion was done
	if (interrupt_called)
	{
		interrupt_called = false;
		was_interrupted = true;
	}
	
	for ( int z = prioritized_job - 1; z < job_list.size(); z++)
	{
		current_burst =  burst_logger[z][2];
		for(int y = 1; y < 22; y ++)
		{
			/*
				context switching. using a truth table:
				(CPU = true) (I/O = false)			how to read:
				Before		Current		result
				 /			 /		 	 X			Was CPU before, current is CPU, no con switch
				 /			 X			 / 			Was CPU before, current is I/O, con switch (or not)
				 X			 /	 		 /			Not I/O before, current is CPU, con switch
				 X			 X			 X			Unreachable
				if the result is true, increment by one. logic is a XORgate
			*/	
			//added 2 parameters: process burst must not be 0, and process was not checked.
			if (job_list[z][y][1] == 2 && was_cpu_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2)
			{
				timer++;
				job_list[z][0][7]++;
				was_cpu_before = false;
			}
			//case 3, might be redundant
			if (job_list[z][y][1] == 1 && was_io_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2)
			{
				timer++;
				job_list[z][0][7]++;
				was_io_before = false;
			}
			while (job_list[z][y][8] != 3 && job_list[z][y][3] != 0)
			{
				/* fetch()
					for every cpu cycle search the ready queue for any arrived jobs.
					should a job arrive at 200, only at the 200th cpu cycle will the job be fetched
					and if the fetched job burst time is less than the current job burst time,
					set the fetched job as prioritized job,
					increment the interrupt counter for the current job,
					flag interrupt called to true,
					reset context switching booleans.
					then goto start of for loop to process the prioritized job.
				*/
				for (int i = 0; i < burst_logger.size(); i++)
				{
					if (burst_logger[i][1] == timer)
					{
						if(burst_logger[i][2] < current_burst)
						{
							//increment current job's interrupt counter
							job_list[z][0][7] ++;
							//save curren job's remaining burst time
							current_burst = burst_logger[z][2];
							prioritized_job = burst_logger[i][0];
							interrupt_called = true;
							was_cpu_before = false;
							was_cpu_before = false;
							timer++;
							goto somewhere;
						}
					}	
				}
				//set the current process to "in cpu" (2)
				job_list[z][y][8] = 2;
				//decrement process burst 
				job_list[z][y][3] -= 1;
				//decrement current job burst
				current_burst--;
				//decrement job wait time (to avoid redoing of the same operation should the scheduler pre-empt)
				job_list[z][0][6] -= 1;
				timer++;
			}
			if (job_list[z][y][1] == 1)
				was_cpu_before = true;
			else
				was_io_before = true;
			//if the process is done se the status to "done" (3)
			job_list[z][y][8] = 3;
		}
		// save complete time to process head
		job_list[z][0][4] = timer;
		// save turnaround time = complete time - arrival
		job_list[z][0][5] = (job_list[z][0][4] - job_list[z][0][2]);
		// save wait time
		job_list[z][0][6] += job_list[z][0][5];
		// add job wait time to avg_wait variable
		avg_wait += job_list[z][0][6];
		
		std::cout << std::endl << "JOB " << job_list[z][0][0] << " arrival: " << job_list[z][0][2] << " Burst time: " << job_list[z][0][3]
				  << " Interrupts: "<< job_list[z][0][7] << " Completion Time: " << job_list[z][0][4] << " tunaround: " << job_list[z][0][5] << " Wait: " << job_list[z][0][6]; 
		
		if(was_interrupted)
		{
			/*
				if the scheduler was once interrupted, find the prioritized job and set its burst time to an obscene number
				sort() in ascending order, then pop back.
				both job list and process logger are in parallel, so both are popped back.
				reset prioritized job to 1, so that the scheduler can run fron the start.
				flag interrupted to false, then goto start.
			*/
			for (int i = 0; i < job_list.size(); i++)
			{
				if (job_list[i][0][0] == prioritized_job)
				{
					job_list[i][0][3] = 999999;
					break;
				}
			}
			std::sort(job_list.begin(),job_list.end(), []
				(const std::vector<std::vector<int>>& lhs, const std::vector<std::vector<int>>& rhs){
				return lhs[0][3] < rhs[0][3];
			});
			
			std::sort (burst_logger.begin(),burst_logger.begin(), []
				(const std::vector<int>& lhs, const std::vector<int>& rhs){
					return lhs[1] < rhs[1];
			});
			
			job_list.pop_back();
			burst_logger.pop_back();
			
			prioritized_job = 1;
			was_interrupted = false;
			goto somewhere;
		}
	}
	
	std::cout << std::endl << "Average wait time = " << avg_wait / 3 << " cycles";
}

void roundR(std::vector<std::vector<std::vector<int>>> job_list)
{
	std::cout << "\n\nRound Robin Scheduling (Quantum = 10 cpu cycle)";
	std::vector<std::vector<int>> process_logger;
	int timer = 1, job_count = 0, quantum = 0 , avg_wait = 0;
	bool was_io_before = false, was_cpu_before = false, robin_interrupt = false;
	/* for reference
		0	1		2		3		4				5		6			7		8
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	State
	1		Job		1		919		0			0			0		7			0
	states:
	0 non arrived (for process heads)
	1 in process / ready
	2 checked
	3 finished
	
	Round Robin is simple, after 10 cycles, a job is switched out to another job, and yada yada yada
	the loop repeats until all 21 processes are done (status 3) (tracked by a process logger vector)
	then remove the job from the 2 vectors
	
	*/
	
	//save job number, job total burst, and process finished (0)
	for (int job = 0; job < job_list.size(); job++)
	{
		int table_number, table_burst;
		table_number = job_list[job][0][0];
		table_burst = job_list[job][0][3];
		std::vector<int> table_row {table_number, table_burst, 0};
		process_logger.push_back(table_row);
	}
	
	//as long as the job list is not empty,
	while (job_list.size() != 0)
	{
		for (int z = 0; z < job_list.size(); z++)
		{
			robin_interrupt = false;
			//reset the process logger to 0 everytime it starts every loop to avoid infinite looping 
			process_logger[z][2] = 0;
			for(int y = 1; y < 22; y ++)
			{
				/*
					context switching. using a truth table:
					(CPU = true) (I/O = false)			how to read:
					Before		Current		result
					 /			 /		 	 X			Was CPU before, current is CPU, no con switch
					 /			 X			 / 			Was CPU before, current is I/O, con switch (or not)
					 X			 /	 		 /			Not I/O before, current is CPU, con switch
					 X			 X			 X			Unreachable
					if the result is true, increment by one. logic is a XORgate
				*/	
				//same as sjf, added 1 parameter: robin interrupt must not be true
				if (job_list[z][y][1] == 2 && was_cpu_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2 && !robin_interrupt)
				{
					timer++;
					job_list[z][0][7]++;
					was_cpu_before = false;
				}
				//case 3
				if (job_list[z][y][1] == 2 && was_io_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2 && !robin_interrupt)
				{
					timer++;
					job_list[z][0][7]++;
					was_io_before = false;
				}
				while (job_list[z][y][8] < 3 && job_list[z][y][3] != 0 && quantum != 11 && !robin_interrupt)
				{
					//set the current process to "in cpu" (2)
					//needs to be in an if statement to prevent reassigning the status
					if (job_list[z][y][8] == 0)
						job_list[z][y][8] = 2;
					//decrement process burst 
					job_list[z][y][3] -= 1;
					//decrement job wait time (to avoid redoing of the same operation should the scheduler pre-empt)
					job_list[z][0][6] -= 1;
					timer++;
					quantum++;
				}
				if (quantum == 11)
				{
					//if the round robin time has ended, flag interrupt as true,
					//reset context switching flags and round robin quantum
					//increment timer and interrupt counter.
					robin_interrupt = true;
					was_io_before = false;
					was_cpu_before = false;
					quantum = 1;
					timer++;
					job_list[z][0][7]++;
				}
				//if process is not finished, AND burst time is 0
				if (job_list[z][y][8] == 2 && job_list[z][y][3] == 0)
				{
					if (job_list[z][y][1] == 1)
						was_cpu_before = true;
					else
						was_io_before = true;
				}
				//if the process is done se the status to "done" (3)
				if (job_list[z][y][3] == 0)
				{
					job_list[z][y][8] = 3;
					process_logger[z][2] += 1;
				}
			}
			// if all processes in a job is done,
			if (process_logger[z][2] == 21)
			{
				// save complete time to process head
				job_list[z][0][4] = timer;
				// save turnaround time = complete time - arrival
				job_list[z][0][5] = (job_list[z][0][4] - job_list[z][0][2]);
				// save wait time
				job_list[z][0][6] += job_list[z][0][5];
				// add job wait time to avg_wait variable
				avg_wait += job_list[z][0][6];
				
				std::cout << std::endl << "JOB " << job_list[z][0][0] << " arrival: " << job_list[z][0][2] << " Burst time: " << job_list[z][0][3]
						  << " Interrupts: "<< job_list[z][0][7] << " Completion Time: " << job_list[z][0][4] << " tunaround: " << job_list[z][0][5] << " Wait: " << job_list[z][0][6]; 
				
				/*
					find the finished job and set its burst time to an obscene number in both tables
					sort() in ascending order, then pop back.
					both job list and process logger are in parallel
				*/
				
				process_logger[z][1] == 9999999;
				job_list[z][0][3] = 999999;
				
				std::sort(job_list.begin(),job_list.end(), []
				(const std::vector<std::vector<int>>& lhs, const std::vector<std::vector<int>>& rhs){
						return lhs[0][3] < rhs[0][3];
				});
				
				std::sort (process_logger.begin(),process_logger.begin(), []
				(const std::vector<int>& lhs, const std::vector<int>& rhs){
					return lhs[1] < rhs[1];
				});
				
				job_list.pop_back();
				process_logger.pop_back();
			}	
		}
	}
	
	std::cout << std::endl << "Average wait time = " << avg_wait / 3 << " cycles";
}

int main()
{	
	std::vector<std::vector<std::vector<int>>> job_vector;
	job_vector = read_file();

	std::cout << std::endl;
	std::cout << "\n\nFirst Come, First Serve Scheduling";
	fcfs(job_vector);
	sjn(job_vector);
	sjf(job_vector);
	roundR(job_vector);
	
	return 0;
}
