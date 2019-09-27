/* CPSC/ECE 3220 Fall 2019
 *
 * pthread code to illustrate a mutex and condition variable solution
 * to the "producer/consumer with bounded buffer" problem
 *
 * the buffer data structure and producer() and consumer() functions
 * are adapted from Oracle Multithreaded Programming Guide, Oct. 2012,
 * docs.oracle.com/cd/E26502_01/html/E35303/sync-21067.html#sync-31
 * Examples 4-11, 4-12, and 4-13
 *
 * I have changed the condition variable names from "more" and "less"
 * in the example code to "not_empty" and "not_full", respectively,
 * so that the condition variable statements can be read as
 *   "wait until <condition> is true" 
 *   "signal that <condition> is true" 
 *
 * compile the program with "gcc -Wall oracle_prod_con.c -pthread"
 * and run with "./a.out <string of P and C characters>", e.g.,
 * "./a.out PPCC"
 *
 * each P creates a producer thread, which adds ten items to the
 * buffer, and each C creates a consumer thread, which removes ten
 * items from the buffer; there should be an equal number of 'P' and
 * 'C' characters so that the program terminates correctly
 *
 * you can also use the helgrind tool, which is part of valgrind,
 * e.g., run "valgrind --tool=helgrind ./a.out CCPPCCPP"
 *
 * you should see a final message of the form:
 * "ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 3460 from 138)"
 * you should ignore the suppressed errors
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<assert.h>

#define MAX_THREADS 20
#define BSIZE 3

typedef struct { 
     char buf[BSIZE]; // circular buffer
     int occupied;    // number of items currently in buffer
     int nextin; 
     int nextout; 
     pthread_mutex_t mutex; 
     pthread_cond_t not_empty; 
     pthread_cond_t not_full;
} buffer_t; 

buffer_t buffer;


void producer(buffer_t *b, char item)
{
    pthread_mutex_lock(&b->mutex);
   
    while(b->occupied >= BSIZE)
        pthread_cond_wait(&b->not_full, &b->mutex);

    assert(b->occupied < BSIZE);

    b->buf[b->nextin++] = item;
    b->nextin %= BSIZE;
    b->occupied++;

    pthread_cond_signal(&b->not_empty);

    pthread_mutex_unlock(&b->mutex);
}

char consumer(buffer_t *b)
{
    char item;

    pthread_mutex_lock(&b->mutex);

    while(b->occupied <= 0)
        pthread_cond_wait(&b->not_empty, &b->mutex);

    assert(b->occupied > 0);

    item = b->buf[b->nextout++];
    b->nextout %= BSIZE;
    b->occupied--;

    pthread_cond_signal(&b->not_full);

    pthread_mutex_unlock(&b->mutex);

    return(item);
}

void *prod_thread( void *thread_id ){
  char item;
  int i;
  for( i = 0; i < 10; i++){
    item = (char)('a' + i);
    printf( "thread %ld produces %c\n", (long)thread_id, item );
    producer( &buffer, item );
  }
  return NULL;
}

void *cons_thread( void *thread_id ){
  char item;
  int i;
  for( i = 0; i < 10; i++){
    item = consumer( &buffer );
    printf( "thread %ld consumes %c\n", (long)thread_id, item );
  }
  return NULL;
}

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
  if( pthread_cond_init( &buffer.not_empty, NULL ) ){
      printf ( "Could not initialize not_empty CV\n");
      return -1;
  }
  if( pthread_cond_init( &buffer.not_full, NULL ) ){
      printf ( "Could not initialize not_full CV\n");
      return -1;
  }

  buffer.nextin = buffer.nextout = 0;
  buffer.occupied = 0;

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
  pthread_cond_destroy( &buffer.not_empty );
  pthread_cond_destroy( &buffer.not_full );

  return 0;
}
