#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <unistd.h>


int debug = 1;
int divisor = 2;
int writers_num = 50;
int readers_num = 5;

pthread_mutex_t mutex;
pthread_mutex_t roomEmpty;
pthread_mutex_t turnstile;
pthread_mutex_t print_sem;
int counter = 0;

pthread_t* writers;
pthread_t* readers;

void print_usage();
void signal_h(int signum);
void* writer_th(void* args);
void* reader_th(void* args);
void at_ex_h(void);
void sig_h(int signum);

long gettid() {
     return syscall(SYS_gettid);
}

#define MAX_MOD_NUM 100
#define MAX_VAL 2048

#define SIZE 1024
int nums[SIZE];

int main(int argc, char** argv){

     signal(SIGINT, sig_h);
     atexit(at_ex_h);
     
     srand(time(NULL));
     
     if(argc != 5){
	  print_usage();
	  return 1;
     } else {
	  debug = atoi(argv[1]);
	  readers_num = atoi(argv[2]);
	  writers_num = atoi(argv[3]);
	  divisor = atoi(argv[4]);
     }

     writers = malloc(sizeof(pthread_t) * writers_num);
     readers = malloc(sizeof(pthread_t) * readers_num);

     pthread_mutex_init(&mutex, NULL);
     pthread_mutex_init(&roomEmpty, NULL);
     pthread_mutex_init(&turnstile, NULL);
     pthread_mutex_init(&print_sem, NULL);
     
     for(int i=0; i<SIZE; i++) nums[i] = 0;

     for(int i=0; i<writers_num; i++){
	  if(pthread_create(&writers[i], NULL, &writer_th, NULL) != 0){
	       perror(NULL);
	       return 0;  
	  }
     }
     
     for(int i=0; i<readers_num; i++){
	  if(pthread_create(&readers[i], NULL, &reader_th, NULL) != 0){
	       perror(NULL);
	       return 0;
	  }
     }


     for(int i=0; i<writers_num; i++)
	  if(pthread_join(writers[i], NULL) != 0){
	       perror(NULL);
	       return 0;
	  }

     for(int i=0; i<readers_num; i++)
	  if(pthread_join(readers[i], NULL) != 0){
	       perror(NULL);
	       return 0;
	  }

     return 0;
}

void print_usage(){
     printf("detail mode [0, 1] readers [num] writters [num] divisor [num]\n");
}

void* writer_th(void* args){
     while(1){
	  
	  pthread_mutex_lock(&turnstile);
	  pthread_mutex_lock(&roomEmpty);


	  // writer code

	  int n = rand() % MAX_MOD_NUM;
	  for(int i=0; i<n; i++){
	       int num = rand() % SIZE;
	       int val = rand() % MAX_VAL;

	       if(debug){
		    pthread_mutex_lock(&print_sem);
		    printf("[%ld] Modifing %d\n", gettid(), num);
		    fflush(stdout);
		    pthread_mutex_unlock(&print_sem);
	       }
	       nums[num] = val;
	  }

	  
	  pthread_mutex_unlock(&turnstile);
	  pthread_mutex_unlock(&roomEmpty);
     }
}

void* reader_th(void* args){
     while(1){
	  
	  pthread_mutex_lock(&turnstile);
	  pthread_mutex_unlock(&turnstile);


	  pthread_mutex_lock(&mutex);
	  counter += 1;
	  if(counter == 1){
	       pthread_mutex_lock(&roomEmpty);
	  }
	  pthread_mutex_unlock(&mutex);

	  // reader code
	  for(int i=0; i<SIZE; i++){
	       if(nums[i] % divisor == 0){
		    if(debug){
			 pthread_mutex_lock(&print_sem);
			 printf("[%ld] Number %d divisable by %d\n", gettid(), nums[i], divisor);
			 fflush(stdout);
			 pthread_mutex_unlock(&print_sem);
		    }
	       }
	  }

     
	  pthread_mutex_lock(&mutex);
	  counter -= 1;
	  if(counter == 0){
	       pthread_mutex_unlock(&roomEmpty);
	  }
	  pthread_mutex_unlock(&mutex);
     }	  
}


void sig_h(int signum){
     exit(0);
}

void at_ex_h(void){
     pthread_mutex_destroy(&mutex);
     pthread_mutex_destroy(&roomEmpty);
     pthread_mutex_destroy(&turnstile);
     free(writers);
     free(readers);
}
