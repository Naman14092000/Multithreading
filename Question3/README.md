# Ober Cab Services

There are N cabs, M riders and K payment servers. M, N and K are user inputs and rest of the variables are randomly generated using `rand` function in C with seed current time. Each rider arrives at a random time and books a cab (either premier or pool). All riders and payment servers are threads. One mutex lock per cab and payment server is used. One mutex lock is used to keep track of remaining riders.

## Cabs
 Implemented using struct `taxi` and for each of the M cabs, a new thread is created. 
 
 - cabStatus denotes the current status of the cab.
 	- waitState : can accept both pool and premier customers.
 	- onRidePremier : currently riding a premier passenger
 	- onRidePoolOne : currently riding a single pool passenger and can accept another pool passenger (priority will be given to it in case a person books a pool cab)
 	- onRidePoolTwo : filled pool cab
 - if any of the rider has completed its ride, cab changes its state.

## Riders
Implemented using struct `rider` and for each of the N riders, a new thread is created.

 - rider's arrival time is simulated using a new thread which sleeps for random time.

 - at any time any number of riders can come.

 - maxwait time is a random number between 10 and 20.

 - ride time is a random number between 1 and 10.
 
 - if the rider is pool, rider->cab_type is 0 else rider->cab_type is 1.

 - if the rider is pool, it searches for a poolOne type cab and if it does not find any it searches for wait cab.

 - he search until it finds a cab or times out if current time exceeds his maxWaitTime(Note: maxwaittime does not include time taken by payment server).

 - after getting a cab he sleeps for ride time and then frees the cab.

 - it then invokes makePayment function and searches for a free server. On finding a free server, it activates that server(changes status from 0 to 1).

 - after completing payment rider exits and increase the variable riders_comp and thread is killed.
 
## Payment Servers
Implemented using array `servers` and for each of the K servers, a new thread is created.

 - it waits for it to be activated i.e. status to change from 0 to 1.

 - it then sleeps for 2 seconds

 - payment is then accepted. status is changed from 1 to 0.

 - if no rider is left, all functions return and all threads are killed. 
