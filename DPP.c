/*
####################################################################################################################################
													DINING PHILOSOPHERS PROBLEM

													COMP3230A DPP.c   															
												    Name:   JAIN Rachit                                              
												    UID:    3035134721                                            
												    Email:  rachit96@hku.hk  
													 
													Development platform: Ubuntu 14.04
													Last Modified: 15 November 2016
													Compilation method: gcc -pthread DPP.c -o DPP                                                


####################################################################################################################################
*/
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct  {	//struct for passing the values to function handler in pthread_create
    int number;	//number of philosphers
    int phil_number; //to identify the philospher
    double timing; //for how long the loop is gonna run
    sem_t *lock; //semaphore for locking the critical section
   
} arg_struct;

typedef enum {	//struct for storing the state of philospher
    THINKING = 0,
    WAITING = 1,
    EAT = 2,
    TERMINATE = 3
} State;

State *philoStates; //array for storing ohilosphers state
pthread_mutex_t *forks; //for storing forks
pthread_t *philosopher; //for storing philosophers number
pthread_t watcher; //watcher thread
int *fork_array; //for stroign the state of forks
void display_state(int N); //funciton declaration for dispaying the philopshers 
const char *state_convert(State philoState); ///function for state convertion

void *func(void * arguments){  //function handling the creation of thread
		arg_struct args = *(arg_struct *)arguments;
		time_t start, end;
	   	double elapsed;  
	   	start = time(NULL);
	   	int terminate = 1; //time flag
	   	double timing = args.timing;
	while (terminate) { //while the time flag is 1
    	end = time(NULL);
     	elapsed = difftime(end, start);
     	if (elapsed <= timing){	//while elapsed time is less than time
			philoStates[args.phil_number] = THINKING; //the philosopher is thinking initially
			usleep((rand()%10000000)+1); //sleeping for random time from 1 microsecond to 10 seconds

			philoStates[args.phil_number] = WAITING; //after thinking for random seconds, the philospher starts //waiting for forks
			sem_wait(args.lock); //locking the critical section to avoid circular deadlock
			pthread_mutex_lock(&forks[args.phil_number]); //acquire one lock
			fork_array[args.phil_number] = args.phil_number; 
			pthread_mutex_lock(&forks[(args.phil_number+1)%args.number]); //acquire another lock
			fork_array[(args.phil_number+1)%args.number] = args.phil_number;
			philoStates[args.phil_number] = EAT; //starts eating
			usleep((rand()%10000000)+1); //eats for random time
			pthread_mutex_unlock(&forks[args.phil_number]); //leaves one fork
			fork_array[args.phil_number] = -1;
			pthread_mutex_unlock(&forks[(args.phil_number+1)%args.number]); //leaves another fork
			fork_array[(args.phil_number+1)%args.number] = -1;
			sem_post(args.lock); //unlocking the critical section
			philoStates[args.phil_number] = THINKING; //changes state for thinking
	 	}
	 	else{
	 		philoStates[args.phil_number] = TERMINATE; //change state to terminate 
	 		pthread_exit(NULL);
	 	}
	}
   return(NULL);

}
void display_state(int N){ //pass agruments: N, 
	int count =0;
		int i, terminated;						//counter for terminated threads

	while(1==1){
    	count=0;
    	usleep(500000);
		//printf("\e[1;1H\e[2J");				//Flush console window
		printf("Philo\t\tState\t\t\tFork\t\tHeld By\n");
		int th=0, waiting=0, eating=0, use=0, available=0;
		terminated=0;
		for(i = 0; i < N; i++) {
				printf("[%d]:\t\t%-10s\t\t", i, state_convert(philoStates[i]));
				if(philoStates[i]==TERMINATE){
					terminated++;
					count++;
				}
				else if(philoStates[i]==EAT)
					eating++;
				else if(philoStates[i]==WAITING)
					waiting++;
				else if(philoStates[i]==THINKING)
					th++;
				if(fork_array[i]==-1){
					printf("[%d]:\t\t%-10s\n", i, "Free"); 
					available++;
				}else{
				 	printf("[%d]:\t\t%d\n", i, fork_array[i]); 
				 	use++;	
				}
		}
		if(count==N){
	    	printf("Th=%d Wa=%d Ea=%d\t\t\t\tUse=%d Avail=%d\n", th, waiting, eating, use, available);
	    	pthread_exit(NULL);
	    }
		printf("Th=%d Wa=%d Ea=%d\t\t\t\tUse=%d Avail=%d\n", th, waiting, eating, use, available);
	}
	
	pthread_exit(NULL);	
}
const char *state_convert(State philoState){ //converting the state of philospher for appropriate printing
	if(philoState == EAT) {
        return "Eating";
    }
    else if(philoState == THINKING) {
        return "Thinking";
    }
    else if(philoState == WAITING){
        return "Waiting";
    }
    else if(philoState == TERMINATE){
        return "Terminated";
    }
    else
    	return "Error"; // error
} 
int main(int argc, char *argv[]){
	if(argc==4){ 
		int N = atoi(argv[1]);
	//the argc needs to be 4 
		int s = atoi(argv[2]);
		srand(s); //seed 
	    double timing = strtol(argv[3], NULL, 10);
	   	int i, id[N];
		pthread_t tid[N];	
		sem_t lock;
		sem_init(&lock, 0, N - 1);
				fork_array = (int *)malloc(N * sizeof(int));
				philosopher = (pthread_t *)malloc(N * sizeof(pthread_t));
				forks = (pthread_mutex_t *)malloc(N * sizeof(pthread_mutex_t));
				philoStates = (State *)malloc(N * sizeof(State));
				for(i = 0; i < N; i++) { //initializing all the forks to be free
					fork_array[i] = -1;			
				}
				for (i = 0; i < N; i++){
					pthread_mutex_init(&forks[i],0);	//creating mutex locks
				}
				for(i=0;i<N;i++){
					arg_struct *args = malloc(sizeof(arg_struct)); 
					args->number = N;
					args->phil_number = i;
					args->timing = timing;
					args->lock = &lock;
					pthread_create(&philosopher[i], NULL, (void *)func, (void * )args); //creating threads for philosphers
				}
				pthread_create(&watcher, NULL, (void *)display_state, (void*) N); //watcher thread
				pthread_join(watcher, NULL); //joing the watcher thread with the mian thread
				pthread_exit(NULL); //exiting the main thread
				return 0;
	}
	else{
		printf("Format: <NumPhilosophers> <Seed> <Time>\n");
		return 0;
	}
}
