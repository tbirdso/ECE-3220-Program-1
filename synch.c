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
	// Allocate node
	node_t *node = (node_t *)malloc(1 * sizeof(node_t));

	// Initialize node flags
	// TODO testme
	pthread_cond_init(&node->partner_available, NULL);
	node->transfer_completed = 0;

	return node;
}

// Add a single node to the producer list
node_t *insert_producer(buffer_t *b, char item)
{
	//TODO testme
	node_t *new_prod = allocate_node();
	printf("Inserting producer node with %c at %p.\n", item, new_prod);

	node_t *prev_tail = b->producer_tail;
	if(prev_tail != NULL) {
		prev_tail->next = new_prod;
	} else {
	// List was empty and now has one node
		b->producer_head = new_prod;
	}
	b->producer_tail = new_prod;

	new_prod->item = item;
	
	return new_prod;
}

// Add a single node to the consumer list
node_t *insert_consumer(buffer_t *b, char *item_ptr)
{
	//TODO testme
	printf("Inserting consumer node\n");
	node_t *new_cons = allocate_node();

	node_t *prev_tail = b->consumer_tail;
	if(prev_tail != NULL) {
		prev_tail->next = new_cons;
	} else {
		//List was empty and now has one node
		b->consumer_head = new_cons;
	}
	b->consumer_tail = new_cons;

	return new_cons;
}

// Get the FIFO producer from the producer list
node_t *remove_producer(buffer_t *b)
{
	printf("Removing producer node.\n");
	node_t *prod = b->producer_head;
	if(b->producer_tail == prod) {
		//List had only one node and is now empty
		b->producer_tail = NULL;
		b->producer_head = NULL;
	} else {
		b->producer_head = prod->next;
	}
	return prod;
}

// Get the FIFO consumer from the consumer list
node_t *remove_consumer(buffer_t *b)
{
	printf("Removing consumer node.\n");
	node_t *cons = b->consumer_head;
	if(b->consumer_tail == cons) {
		// List had only one node and is now empty
		b->consumer_tail = NULL;
		b->consumer_head = NULL;
	} else {	
		b->consumer_head = cons->next;
	}
	return cons;
}


// synchronous functions that must be coded using chapter 5 guidelines

void put(buffer_t *b, char item)
{
	node_t *prod_node;

	assert(b != NULL);

	// 1. Acquire lock
	pthread_mutex_lock(&b->mutex);
	printf("set() acquired lock.\n");

	// 2. Check to see if consumer list is non-empty.
	if(b->consumer_head != NULL) {
	//	a. Transfer value of item into consumer node
	//	b. Update consumer node CV

		node_t *cons = remove_consumer(b);
		cons->item = item;
		cons->transfer_completed = 1;
		pthread_cond_signal(&cons->partner_available);

	} else {
	//	a. insert a new node into the producer list
	//	b. wait for CV to update where transfer is completed
	//	c. destroy transfer CV and free node

		node_t *prod = insert_producer(b, item);

		while(prod->transfer_completed == 0) {
			pthread_cond_wait(&prod->partner_available, &b->mutex);
		}

		pthread_cond_destroy(&prod->partner_available);
		free(prod);
	}

	pthread_mutex_unlock(&b->mutex);

	return;
}

char get(buffer_t *b)
{
	//TODO: testme
	node_t *cons_node;
	char item;

	assert(b != NULL);

	// 1. Acquire lock
	pthread_mutex_lock(&b->mutex);
	printf("get() acquired lock.\n");

	// 2. Check on CV to see if producer list is non-empty.
	if(b->producer_head != NULL) {
	//	a. Transfer value of item from producer nodee
	//	b. Update producer node CV

		node_t *prod = remove_producer(b);
		item = prod->item;
		printf("get() found item %c at %p.\n", item, prod);
		prod->transfer_completed = 1;
		pthread_cond_signal(&prod->partner_available);

	} else {
	//	a. insert a new node into consumer list
	//	b. wait for CV to update where transfer is completed
	//	c. output item value (?)
	//	d. destroy transfer CV and free node
	
		node_t *cons = insert_consumer(b, &item);

		while(cons->transfer_completed == 0) {
			pthread_cond_wait(&cons->partner_available, &b->mutex);
		}
		
		item = cons->item;

		pthread_cond_destroy(&cons->partner_available);
		free(cons);
	}

 	pthread_mutex_unlock(&b->mutex);

	return item;
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
