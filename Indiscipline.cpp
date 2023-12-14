#include <iostream> 
#include <algorithm>
#include <vector>
#include <fstream>
#include <iomanip>
#include <chrono>

long long timeUS() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }
long long start, end;

std::vector<std::vector<std::vector<int>>> read_file()
{
	std::ifstream file;
	file.open("g6.csv");
	
	std::vector<std::vector<std::vector<int>>> job_list;
	
	if(file.is_open())
		std::cout << "File is open, reading file...";
	else
	{
		std::cerr << "Can't open file! Exiting..." << std::endl;
		return job_list;
	}
	
	int job_number = 1, process_number = 1, process_type, arrival, burst, total_burst;
	std::string job_Name, proc_name;
	
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
		// create the process_head vector and push back into the column
		std::vector<int> process_head {job_number, 0, arrival, 0,0,0,0,0,0};
		job.push_back(process_head);
		
		// skip the '\n' char
		getline(file, proc_name);
		
		for (int i = 0; i < 122 ; i++)
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
		// skip the '\n' char
		getline(file, proc_name);
		
		job[0][3] = total_burst;
		
		// push back the column into the row of columns
		job_list.push_back(job);
		job_number++;
	}
	file.close();
	return job_list;
}

void roundR(std::vector<std::vector<std::vector<int>>> job_list, int max_quantum)
{
	std::cout << "\n\n \t\t\t\t\tRound Robin Scheduling (Quantum = " << max_quantum << " cpu cycles)";
	std::vector<std::vector<int>> process_logger;
	int timer = 1, job_interrupts = 0, quantum = 1 , avg_wait = 0, avg_turnaround = 0,  total_jobs = job_list.size();
	bool was_io_before = false, was_cpu_before = false, robin_interrupt = false;
	
	/* for reference
	Process heads (job vector element 0)
		0	1		2		3		4				5		6			7		8
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	no of processes finished
	1		Job		1		919		0			0			0		7			(ranges from 0 to 122)
	
	Processes (vector element 1 - 122)
		0	1		2		3		4				5		6			7		8
	Number	Type	Arrival	Burst	Complete	Turnaround	Wait	Interrupt	State
	1		CPU/IO	unused	43		unused		unused		unused	unused		0
																				0 non arrivabene
																				1 in process / ready
																				2 checked
																				3 finished
	
	Round Robin is simple, after n cycles, a job is switched out to another job, and yada yada yada
	the loop repeats until all 122 processes are done (status 3)
	then remove the job from the vector
	
	*/
	
start = timeUS();
	
	//as long as the job list is not empty,
	while (job_list.size() != 0)
	{
		for (int z = 0; z < job_list.size(); z++)
		{
			robin_interrupt = false;
			//reset the process logger to 0 everytime it starts every loop to avoid infinite looping 
			job_list[z][0][8] = 0;
			if ( timer >= job_list[z][0][2] )
			{
				for(int y = 1; y < 123; y ++)
				{
					if (!robin_interrupt)
					{
						/*
							context switching. using a truth table:
							(CPU = true) (I/O = false)			how to read:
							Before		Current		result
							 /			 /		 	 X			Was CPU before, current is CPU, no con switch
							 /			 X			 / 			Was CPU before, current is I/O, con switch
							 X			 /	 		 /			Not I/O before, current is CPU, con switch
							 X			 X			 X			Was I/O before, current is I/O, no con switch
							if the result is true, increment by one. logic is a XORgate
						*/	
						//same as srt, added 1 parameter: robin interrupt must not be true
						if (job_list[z][y][1] == 2 && was_cpu_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2)
						{
							timer++;
							job_list[z][0][7]++;
							was_cpu_before = false;
						}
						if (job_list[z][y][1] == 1 && was_io_before == true && job_list[z][y][3] != 0 && job_list[z][y][8] < 2)
						{
							timer++;
							job_list[z][0][7]++;
							was_io_before = false;
						}
						while (job_list[z][y][8] < 3 && job_list[z][y][3] != 0 && quantum != max_quantum + 1)
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
					
						if (quantum == max_quantum + 1)
						{
							//if the round robin time has ended, flag interrupt as true,
							//reset context switching flags and round robin quantum
							//increment timer and interrupt counter.
							robin_interrupt = true;
							was_io_before 	= false;
							was_cpu_before 	= false;
							quantum = 1;
							timer++;
							job_list[z][0][7]++;
							job_interrupts++;
						}
						//if process is not finished, AND burst time is 0
						if (job_list[z][y][8] == 2 && job_list[z][y][3] == 0)
						{
							if (job_list[z][y][1] == 1)
								was_cpu_before	= true;
							else
								was_io_before	= true;
						}
						//if the process is done se the status to "done" (3)
						if (job_list[z][y][3] == 0)
						{
							job_list[z][y][8] = 3;
							job_list[z][0][8] += 1;
						}
					}
				}
			}
			// if all processes in a job is done,
			if (job_list[z][0][8] == 122)
			{
				// save complete time to process head
				job_list[z][0][4] = timer;
				// save turnaround time = complete time - arrival
				job_list[z][0][5] = (job_list[z][0][4] - job_list[z][0][2]);
				// save wait time
				job_list[z][0][6] += job_list[z][0][5];
				// add job wait time to avg_wait variable
				avg_wait += job_list[z][0][6];
				avg_turnaround +=  job_list[z][0][5];
				
				std::cout << std::endl << "JOB " << std::setw(2)<< job_list[z][0][0] << " arrival: " << std::setw(4) << job_list[z][0][2] << " Burst time: " << job_list[z][0][3]
						  << " Interrupts: " << std::setw(3) << job_list[z][0][7] << " Completion Time: " << job_list[z][0][4] << " tunaround: " << job_list[z][0][5] << " Wait: " << job_list[z][0][6]; 
			
				/*
					find the finished job and set its job number to an obscene number in both tables
					sort() in ascending order, then pop back.
					both job list and process logger are in parallel
				*/
				
				job_list[z][0][0] = 999999;
				
				std::sort(job_list.begin(),job_list.end(), []
				(const std::vector<std::vector<int>>& lhs, const std::vector<std::vector<int>>& rhs){
						return lhs[0][0] < rhs[0][0];
				});
				
				job_list.pop_back();
				z--;
			}	
		}
	}
end = timeUS();

	std::cout << "\n\nAverage wait time = " << avg_wait / total_jobs << " cycles";
	std::cout << "\nAverage Turnaround time = " << avg_turnaround / total_jobs << " cycles";
	std::cout << "\nRun time: " << (end - start) << " milliseconds";
	std::cout << "\nTotal round robin pre-emptions: " << job_interrupts ;
}

int main()
{	
	std::vector<std::vector<std::vector<int>>> job_vector = read_file();
	if (job_vector.size() == 0)
		return 0;
	
	int choose;
	//the usual input validation thingies
	while (true)
	{
	    std::cout << "\n\nPlease choose a time quantum: \n"
	              " (5,10,15,20,25,30,35,40,45,50,55,60) \n"
	              " (enter 0 to exit) \n"
	              "--->";
	    try
	    {
	        std::cin >> choose;
	        if (!std::cin)
	            throw std::ios_base::failure("\n[ERROR] Please enter a number.");
	
	        if (choose == 0)
	        {
	            std::cout << "babai";
	            break;
	        }
	        else if (choose < 5 || choose > 60 || choose % 5 != 0)
	            throw std::ios_base::failure("\n[ERROR] Please enter a valid number.");
	
	        roundR(job_vector, choose);
	    }
	    catch (const std::ios_base::failure& e)
	    {
	        std::cout << e.what() << std::endl;
	        std::cin.clear();
	        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	    }
	}
//this one right here displays everyting at once.
//	for (int max_quantum = 5; max_quantum < 65; max_quantum += 5)
//		roundR(job_vector, max_quantum);
	
	return 0;
}
