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


typedef         uintmax_t       tp_tid_t;
typedef struct  thread_pool     tp_t;
typedef struct  tp_data         tp_thread_t;
typedef void*   (*tp_func)      ( void* args );


extern tp_t* std_tp;


tp_t*       tp_init( size_t thread_num );
void        tp_destroy_p( tp_t* tp );
tp_tid_t    tp_add_p( tp_t* tp, bool ret, tp_func func, void* args );
bool        tp_query_p( tp_t* tp, tp_tid_t tid, tp_thread_t* thread );
void        tp_clear_p( tp_t* tp );
tp_status_t tp_status( tp_thread_t data );
void*       tp_res( tp_thread_t data );


#define tp_destroy()                tp_destroy_p(std_tp)
#define tp_add( ret, func, args )   tp_add_p( std_tp, ret, func, args )
#define tp_query( tid, thread )     tp_query_p( std_tp, tid, thread )
#define tp_clear()                  tp_clear_p(std_tp)


#define TP_


#endif  // __TP_H__