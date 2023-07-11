#include "tp.h"


struct tp_data
{
    tp_tid_t            tid;
    bool                ret;
    tp_func             func;
    void*               args;
    void*               res;
    _Atomic tp_status_t status;
};


typedef struct tp_node
{
    tp_thread_t         func;
    struct tp_node*     next;
} tp_node_t;


typedef struct tp_list
{
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    tp_node_t*          head;
    tp_node_t*          tail;
} tp_list_t;


struct thread_pool
{
    atomic_uintmax_t    tid_counter;
    atomic_bool         should_end;
    pthread_t*          worker;
    size_t              worker_count;
    tp_list_t           waiting;
    tp_list_t           done;
};


tp_t* std_tp = NULL;


static void* tp_worker( void* args )
{
    tp_t* tp = args;
    tp_node_t* curr;

    while ( tp->should_end == false )
    {
        // fetch job
        pthread_mutex_lock( &tp->waiting.mutex );
        while ( tp->waiting.head == NULL )
        {
            pthread_cond_wait( &tp->waiting.cond, &tp->waiting.mutex );
            if ( tp->should_end == true )
            {
                pthread_mutex_unlock( &tp->waiting.mutex );
                return NULL;
            }
        }
        curr = tp->waiting.head;
        tp->waiting.head = curr->next;
        pthread_mutex_unlock( &tp->waiting.mutex );
        curr->next = NULL;

        // run job
        atomic_store( &curr->func.status, TP_RUNNING );
        if ( curr->func.ret == true )
        {
            curr->func.res = curr->func.func( curr->func.args );
        }
        else
        {
            curr->func.func( curr->func.args );
        }

        // clean up
        if ( tp->should_end == true )
        {
            free(curr);
            return NULL;
        }
        if ( curr->func.ret == true )
        {
            atomic_store( &curr->func.status, TP_DONE );
            pthread_mutex_lock( &tp->done.mutex );
            if ( tp->done.head == NULL )
            {
                tp->done.head = tp->done.tail = curr;
            }
            else
            {
                tp->done.tail->next = curr;
                tp->done.tail = curr;
            }
            pthread_mutex_unlock( &tp->done.mutex );
        }
        else
        {
            free(curr);
        }
    }
    return NULL;
}


static inline tp_tid_t tp_next_tid( tp_t* restrict tp )
{
    return atomic_fetch_add( &tp->tid_counter, 1 );
}


tp_t* tp_init( size_t thread_num )
{
    tp_t* tp = NULL;
    tp = malloc( sizeof (tp_t) );
    *tp = (tp_t)
    {
        .worker         = malloc( sizeof (pthread_t) * thread_num ),
        .worker_count   = thread_num,
    };
    pthread_mutex_init( &tp->waiting.mutex, NULL );
    pthread_mutex_init( &tp->done.mutex, NULL );
    pthread_cond_init( &tp->waiting.cond, NULL );

    for ( size_t i = 0; i < tp->worker_count; i++ )
    {
        pthread_create( &tp->worker[i], NULL, tp_worker, tp );
        // pthread_detach( tp->worker[i] );
    }

    if ( std_tp == NULL )
    {
        std_tp = tp;
    }

    return tp;
}


void tp_destroy_p( tp_t* tp )
{
    atomic_store( &tp->should_end, true );
    pthread_cond_broadcast( &tp->waiting.cond );
    for ( size_t i = 0; i < tp->worker_count; i++ )
    {
        pthread_join( tp->worker[i], NULL );
    }
    pthread_mutex_destroy( &tp->waiting.mutex );
    pthread_mutex_destroy( &tp->done.mutex );
    pthread_cond_destroy( &tp->waiting.cond );
    // pop both queue
    tp_node_t* temp;
    tp_node_t* curr = tp->waiting.head;
    // printf( "waiting.head: 0x%p\n", curr );
    while ( curr != NULL )
    {
        temp = curr->next;
        free( curr );
        curr = temp;
    }
    curr = tp->done.head;
    while ( curr != NULL )
    {
        temp = curr->next;
        free( curr );
        curr = temp;
    }
    free( tp->worker );
    free(tp);
}


tp_tid_t tp_add_p( tp_t* tp, bool ret, tp_func func, void* args )
{
    pthread_mutex_lock( &tp->waiting.mutex );
    if ( tp->waiting.head == NULL )
    {
        tp->waiting.head = tp->waiting.tail = malloc( sizeof (tp_node_t) );
    }
    else
    {
        tp->waiting.tail->next = malloc( sizeof (tp_node_t) );
        tp->waiting.tail = tp->waiting.tail->next;
    }
    tp_tid_t tid = tp_next_tid(tp);
    *(tp->waiting.tail) = (tp_node_t)
    {
        .func = 
        {
            .tid    = tid,
            .ret    = ret,
            .func   = func,
            .args   = args,
            .status = TP_WAITING,
        },
    };
    pthread_mutex_unlock( &tp->waiting.mutex );
    pthread_cond_signal( &tp->waiting.cond );
    return tid;
}


bool tp_query_p( tp_t* tp, tp_tid_t tid, tp_thread_t* thread )
{
    pthread_mutex_lock( &tp->waiting.mutex );
    for ( tp_node_t* curr = tp->waiting.head; curr != NULL; curr = curr->next )
    {
        if ( curr->func.tid == tid )
        {
            *thread = curr->func;
            pthread_mutex_unlock( &tp->waiting.mutex );
            return true;
        }
    }
    pthread_mutex_unlock( &tp->waiting.mutex );

    pthread_mutex_lock( &tp->done.mutex );
    for ( tp_node_t *curr = tp->done.head, *last = NULL; curr != NULL; curr = curr->next )
    {
        if ( curr->func.tid == tid )
        {
            *thread = curr->func;
            if ( curr == tp->done.head )
            {
                tp->done.head = NULL;
                free(curr);
            }
            else
            {
                last->next = curr->next;
                free(curr);
            }
            pthread_mutex_unlock( &tp->done.mutex );
            return true;
        }
        last = curr;
    }
    pthread_mutex_unlock( &tp->done.mutex );
    return false;
}


void tp_clear_p( tp_t* tp )
{
    pthread_mutex_lock( &tp->done.mutex );

    tp_node_t* curr = tp->done.head;
    tp_node_t* next;
    while ( curr != NULL )
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    pthread_mutex_unlock( &tp->done.mutex );
}


tp_status_t tp_status( tp_thread_t thread )
{
    return thread.status;
}


void* tp_res( tp_thread_t thread )
{
    return thread.res;
}


