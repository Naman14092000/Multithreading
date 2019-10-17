# Automated Biryani Serving

There are N robot chefs, M serving tables and K students. Each of them is a thread. There is a global mutex lock on total number of students and one mutex lock for each table.
N, M and K are user inputs and rest of the variables are randomly generated using `rand()` function in C with seed current time.

## Robot Chefs 
Implemented using struct `robot` and for each of the M chefs, a new thread is created.

 - preparation time of robot is simulated by `sleep(<random int between 2 and 5>)`

 - it then invokes biryani_ready function.

 - inside biryani_ready function it continously checks for all tables and whenever it finds a table with status 0, it locks the table and refill that table (makes its status 2). this is done for all its vessels.

## Serving Tables
Implemented using struct `vessel` and for each of the N tables, a new thread is created. 

 - it waits for it to be filled (status to change from 0 to 1)

 - it then creates random number of slots between 1 to min(10,capacity).

 - it then invokes ready_to_serve function

 - inside the function it loops until its slots becomes zero and come back to vessel_wait. 

 - if capacity is zero then it changes state to 0.

 - it can now be rediscovered by robot chefs.

 - it returns when all students ate the food.
 
## Students
Implemented using struct `stud` and for each of the K students, a new thread is created.

 - each student arrives at a random time.

 - it then invokes student_wait() function.

 - inside the function it searches for a table with available slot till it finds one.

 - it then occupies that slot and wait for all slots to be occupied.

 - after all slots are occupied, student eats the food and decrease the slots and capacity of respective container.

 - whenever students_rem reaches zero, all the functions return and all threads are killed implying simulation is over.

>Only one mutex lock is used per table and one global mutex lock is used to count the number of students remaining.
