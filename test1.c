#include "src/tp.h"
#include <unistd.h>


void* foo( void* args, const tp_t* tp, tp_tid tid )
{
    (void) tp;
    (void) tid;
    int num = *(int*)args;
    char str[10];
    snprintf( str, 9, "%d", num );
    puts(str);
    return NULL;
}


int main( void )
{
    tp_init( 7 );

    int arr[5];

    for ( int i = 0; i < 5; i++ )
    {
        arr[i] = i;
        tp_async( true, foo, &arr[i] );
    }

    for ( int i = 0; i < 5; i++ )
    {
        arr[i] = i;
        tp_async( false, foo, &arr[i] );
    }

    tp_wait();

    return 0;
}