/* Feel free to use this example code in any way
   you see fit (Public Domain) */



#include "httphandler.h"

int main ()
{
    RestApiListener listener(8080);
    listener.start();

    listener.stop();
    return 0;
}
