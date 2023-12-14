# C++ OS scheduler
Made for a course project. Doesn't exactly mirror an actual OS scheduler, but the concept is pretty much the same. Essentially these programs are number crunchers decrementing a job once per loop until the whole job list is done. So while it does have the task states (start, ready, running, waiting, and terminated), both the IO and Jobs are processed at the same place.
Contains 4 scheduling algorithms: First in First out (FIFO), Shortest Job Next (SJN), Shortest Job First (SJF) and Round Robin (RR)
# In this repo,
There are 3 cpp files.
- KC: The first iteration of the scheduler. Allows the user to pick which scheduler to run EXCEPT RR. Buggy.
- reconstruKction - The second iteration of the scheduler. Allows the user to pick which scheduler to run.
- indiscipline - The round-robin only version. allows the user to enter a time quantum in increments of 5.
KC and reconstruKction uses joblist.csv and indiscipline uses g6.csv.
# Doubts
Now im not sure if any of these are correct. The lecturer did not send a correct version (neither did she correct us) but based on my intuition and studies it should be correct. Someone else's code might be more correct than mine, especially the SJN and RR algos.
