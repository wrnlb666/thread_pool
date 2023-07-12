#ifndef __TP_H__
#define __TP_H__


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>


typedef enum tp_status
{
    TP_WAITING,
    TP_RUNNING,
    TP_DONE,
} tp_status_t;


typedef         uintmax_t       tp_tid;
typedef struct  thread_pool     tp_t;
typedef struct  tp_data         tp_thread_t;
typedef void*   (*tp_func)      ( void* args, const tp_t* tp, tp_tid tid );


extern tp_t* std_tp;


tp_t*       tp_init( size_t thread_num );                                   // initialize thread pool with certain amount of threads
void        tp_destroy_p( tp_t* tp );                                       // clear all tasks in the waiting queue, let the current tasks finishes, destroy the pool
void        tp_wait_p( tp_t* tp );                                          // let all the tasks in the thread pool finish, then destroy the pool
tp_tid      tp_async_p( tp_t* tp, bool ret, tp_func func, void* args );     // submit a task to the pool
bool        tp_query_p( tp_t* tp, tp_tid tid, tp_thread_t* thread );        // search for a thread. Return true if found, return false if not found(also finished non-ret task). Result will be `thread`
void        tp_clear_p( tp_t* tp );                                         // clear the done queue of a thread pool, will be unable to fetch the thread and get result from previously finished threads
tp_status_t tp_status( tp_thread_t data );                                  // get the status information of a thread
void*       tp_res( tp_thread_t data );                                     // get the return value of a thread


#define tp_destroy()                tp_destroy_p(std_tp)
#define tp_wait()                   tp_wait_p(std_tp)
#define tp_async( ret, func, args ) tp_async_p( std_tp, ret, func, args )
#define tp_query( tid, thread )     tp_query_p( std_tp, tid, thread )
#define tp_clear()                  tp_clear_p(std_tp)



#define TP_ASYNC_FAIL               UINTMAX_MAX




#endif  // __TP_H__