#include "src/tp.h"
#include <unistd.h>


atomic_int counter = 0;


void* foo( void* args )
{
    (void) args;
    char str[10];
    snprintf( str, 9, "%d", counter++ );
    puts(str);
    return NULL;
}


int main( void )
{
    tp_init( 5 );

    for ( size_t i = 0; i < 5; i++ )
    {
        tp_add( true, foo, NULL );
    }


    for ( size_t i = 0; i < 5; i++ )
    {
        tp_add( false, foo, NULL );
    }

    sleep(1);
    tp_destroy();

    return 0;
}