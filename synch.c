/* CPSC/ECE 3220 Fall 2019
 *
 * Programmer:	Tom Birdsong
 * Due Date:	10/8/2019
 * Assignment:	Project 1
 * pthread code for synchronous buffer
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<assert.h>

#define MAX_THREADS 20


typedef struct node_struct {
     pthread_cond_t partner_available;
     char item;
     char *item_ptr;
     int transfer_completed;
     struct node_struct *next;
} node_t;
     
typedef struct { 
     node_t *producer_head, *producer_tail,
            *consumer_head, *consumer_tail;
     pthread_mutex_t mutex; 
} buffer_t; 

buffer_t buffer;


// helper functions - these are called only when a lock is held
// so these functions do not need lock and unlock themselves

// Dynamically allocate a single node to put in the buffer
node_t * allocate_node()
{
	node_t *node = (node_t *)malloc(1 * sizeof(node_t));
	// TODO: init CV
	return node;
}

// Add a single node to the producer list
node_t *insert_producer(buffer_t *b, char item)
{
  // your code
	//TODO
	// 3. Traverse list
	// 4. Create producer node
	// 5. Insert at tail of list
}

// Add a single node to the consumer list
node_t *insert_consumer(buffer_t *b, char *item_ptr)
{
  // your code

	//TODO
	// 3. Traverse list
	// 4. Create consumer node
	// 5. Insert at tail of list
}

// Get the FIFO producer from the producer list
void remove_producer(buffer_t *b)
{
  // your code
	//TODO
	// 3. Get a pointer to the first node
	// 4. Get a pointer to the second node
	// 5. Set the second node as the head of the list
	// 6. Return a pointer from the list
}

// Get the FIFO consumer from the consumer list
void remove_consumer(buffer_t *b)
{
  // your code
}


// synchronous functions that must be coded using chapter 5 guidelines

void put(buffer_t *b, char item)
{
  // your code
	node_t *prod_node;
	pthread_mutex_lock(&(buffer.mutex));

	// 1. Acquire lock
	// 2. Check on CV to see if consumer list is non-empty. If so,
	//	a. Transfer value of item into consumer node
	//	b. Update consumer node CV
	// 3. If consumer list is empty
	//	a. insert a new node into the producer list
	//	b. wait for CV to update where transfer is completed
	//	c. destroy transfer CV and free node
	pthread_mutex_unlock(&(buffer.mutex));
}

char get(buffer_t *b)
{
  // your code
	node_t *cons_node;
	pthread_mutex_lock(&buffer.mutex);
	// 1. Acquire lock
	// 2. Check on CV to see if producer list is non-empty. If so,
	//	a. Transfer value of item from producer node
	//	b. Update producer node CV
	// 3. If producer list is empty
	//	a. insert a new node into consumer list
	//	b. wait for CV to update where transfer is completed
	//	c. output item value (?)
	//	d. destroy transfer CV and free node
	pthread_mutex_unlock(&buffer.mutex);
}


// thread functions to call the synchronous put and get functions

void *prod_thread( void *thread_id ){
  char item;
  item = (char)('a' + (long)thread_id);
  printf( "thread %ld produces %c\n", (long)thread_id, item );
  put( &buffer, item );
  printf( "thread %ld returns from put()\n", (long)thread_id );
  return NULL;
}

void *cons_thread( void *thread_id ){
  char item;
  printf( "        thread %ld calls get()\n", (long)thread_id );
  item = get( &buffer );
  printf( "        thread %ld consumes %c\n", (long)thread_id, item );
  return NULL;
}


// driver code

int main( int argc, char **argv ){
  pthread_t threads[MAX_THREADS];
  int i;
  int rc;
  int thread_count = 0;

  if( argc != 2 ){
    printf( "producer-consumer string should be provided\n" );
    printf( "as a command argument, e.g., PPCCCP\n" );
    exit( 0 );
  }

  if( pthread_mutex_init( &buffer.mutex , NULL ) ){
      printf ( "Could not initialize mutex\n");
      return -1;
  }

  buffer.producer_head = buffer.producer_tail = NULL;
  buffer.consumer_head = buffer.consumer_tail = NULL;

  for( i = 0; i < strlen( argv[1] ); i++ ){

    if( thread_count == MAX_THREADS ) break;
  
    if( argv[1][i] == 'P'){
      rc = pthread_create( &threads[thread_count], NULL, &prod_thread,
                           (void *)((long)thread_count) );
    }else if( argv[1][i] == 'C'){
      rc = pthread_create( &threads[thread_count], NULL, &cons_thread,
                           (void *)((long)thread_count) );
    }
    if( rc ){ printf( "** could not create thread %d\n", i ); exit( -1 ); }
    thread_count++;
  }

  for( i = 0; i < thread_count; i++ ){
    rc = pthread_join( threads[i], NULL );
    if( rc ){ printf( "** could not join thread %d\n", i ); exit( -1 ); }
  }

  pthread_mutex_destroy( &buffer.mutex );

  return 0;
}
