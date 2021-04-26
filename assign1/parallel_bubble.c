/* Even-Odd Sequential Bubble Sort
 * Author: Kartik Gopalan
 * Date: Aug 31 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>



#define MAX_COUNT 1000 // Max integers to sort
#define MAX_NUM 100 // Generate numbers between 0 and MAX_NUM
#define MAX_P 100
#define SHM_SIZE 1024000 
// Uncomment the following line to turn on debugging messages
//#define DEBUG

long number[MAX_COUNT];
int N; // Number of integers to sort
int P;


typedef struct Data{

int array[MAX_COUNT];
int pids[MAX_P];
int done[MAX_P];
int swap_counter[MAX_P];
int exit_now;
int pass;

}Data;

Data *data;
// generate N random numbers between 0 and MAX_NUM
void generate_numbers()
{
	int i;
	
	srandom(time(NULL));

	for(i=0; i<N; i++) {
		data->array[i] = random()%MAX_NUM;
	}
	
}


void done_array(int process,int value){
	for(int i=0;i<process;i++){
		data->done[i] = value;
	}
}

void set_counter(int process,int value){
	for(int i=0;i<process;i++){
		data->swap_counter[i] = value;
	}
}

void print_numbers() 
{
	int i;

	for(i=0; i<N; i++){
		printf("%ld ", data->array[i]);
	}	
	printf("\n");
}

int compare_and_swap(int i, int j) 
{
#ifdef DEBUG
	fprintf(stderr,"i %d j %d\n", i, j);
#endif
	// assert ( i<N );
	// assert ( j<N );

	if( data->array[i] > data->array[j]) {
		long temp =  data->array[i] ;
		data->array[i] = data->array[j] ;
		data->array[j] = temp;
		return 1;
	}

	return 0;
}






// even-odd pass bubbling from start to start+n
int bubble(int start, int n, int id, int pass) 
{
#ifdef DEBUG
	fprintf(stderr, "start %d n %d pass %d\n", start, n, pass);
#endif

	if(start + n > N){
		n = N - start;
	}
	while (data->done[id] != 1)
	{
	int swap_count = 0;
	int next = start;

	assert (start < N-1); // bug if we start at the end of array

	if (pass) { // sort odd-even index pairs
		if ( !(next % 2) ) 
			next = next + 1;
	} else  { // sort even-odd index pairs
		if (next % 2) 
			next = next + 1;
	}

	while ( (next+1) < (start+n) ) {
		swap_count += compare_and_swap(next, next+1);
		next+=2;
	}
	data->swap_counter[id] = swap_count;
	data->done[id] = 1;
	}

}

void bubble_sort(int start, int n, int id, int pass) 
{
	int last_count, swap_count = N;


#ifdef DEBUG
	print_numbers();
#endif

	do {
		last_count = swap_count;
		swap_count = bubble(start,n, id, pass); // 0 for single-process sorting
#ifdef DEBUG
		print_numbers();
		fprintf(stderr,"last_count %d swap_count %d\n", last_count, swap_count);
#endif
		// pass = 1-pass;
	} while(data->done[id] != 1);
}

void shm_create(){

       key_t key;
       int shmid;

       int mode;

       /* make the key: */
       if ((key = ftok("shm_file", 'X')) < 0) {
            perror("ftok");
            exit(1);
       }
	   
       /* create the shared memory segment: */
       if ((shmid = shmget(key, (1*(sizeof(Data))), 0644 | IPC_CREAT )) < 0) {
            perror("shmget");
            exit(1);
       }
	   

	   /* attach to the segment to get a pointer to it: */
        data = (Data *)shmat(shmid, (void *)0, 0);
        if ( data== (Data *)(-1)) {
            perror("shmat");
            exit(1);
        }
}

int
summationswap( int P){
	int res = 0;
	for(int i = 0; i < P; i++){
		res+=data->swap_counter[i];
	}
	return res;
}


int
summationdone( int P){
	int res = 0;
	for(int i = 0; i < P; i++){
		res+=data->done[i];
	}
	return res;
}





int main(int argc, char *argv[])
{


	if( argc != 3) {
		fprintf(stderr, "Usage: %s N\n", argv[0]);
		return 1;
	}
	

	N = strtol(argv[1], (char **)NULL, 10);
    P = strtol(argv[2], (char **)NULL, 10);

	if( (N < 2) || (N > MAX_COUNT) ) {
		fprintf(stderr, "Invalid N. N should be between 2 and %d.\n", MAX_COUNT);
		return 2;
	}

    if((P > 5) || (P < 2)){
        fprintf(stderr, "Do not include processes more than 5\n");
		exit(0);
    }


	int status;
	int ret;
	
	//STRUCT DEFINITION

	//Creating Shared memory
	shm_create();

		
	/* detach from the segment: */
    
	int looper;
	
	int temp_pid;
	int start,end, temp_end;
	generate_numbers();
	print_numbers();
	done_array(P,0);
	set_counter(P,0);
	
	int range = N/P;
	data->exit_now = 0;
	data->pass = 0;
	looper=0;
	while (data->exit_now != 1)
	{
		looper+=1;
		
		for(int i=0; i<P; i++){
        temp_pid = fork();
        
        if (temp_pid< 0) { 
            perror("fork failed:"); 
            exit(1); 
        } 

        if (temp_pid== 0) { // Child executes this block
			int start = i*range;
			int n = range+1;
			bubble_sort(start,n,i,data->pass);
            exit(0); 
        } 

        if (temp_pid > 0) { //Parent executes this block
			data->pids[i] = temp_pid;
			if(summationdone(P) == P){
				done_array(P,0);
				data->pass = 1 - data->pass;
			}
			if((looper%P)==0){
				if(summationswap(P)== 0){
					data->exit_now = 1;
				}
			}
		} 
		}
	}
	


	for(int i=0; i<P; i++){
		wait(NULL);
	}
	for(int i=0; i<P; i++){
		printf("stored pids %d: %d\n",i,data->pids[i]);
	}

	
	//fprintf(stderr, "Generating.\n");
	

	//fprintf(stdout, "Generated sequence is as follows:\n");
	

	//fprintf(stderr, "Sorting.\n");
	//bubble_sort();

	//fprintf(stdout, "Sorted sequence is as follows:\n");
	print_numbers();
	fprintf(stderr, "Done.\n");

	//getting stored pid from struct

	

	

	return 0;
}

