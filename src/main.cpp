/* Feel free to use this example code in any way
   you see fit (Public Domain) */



#include "httphandler.h"

int main ()
{
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, nullptr, nullptr,
                             &answer_to_connection, nullptr,
                             MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                             nullptr, MHD_OPTION_END);
  if (nullptr == daemon)
    return 1;

  (void) getchar ();

  MHD_stop_daemon (daemon);

  return 0;
}
