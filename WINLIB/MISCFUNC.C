/*
**  miscfunc.c - miscellaneous functions
*/

#include <genmess_h>
#include <genbench_h>
#include <genvideo_h>

extern  int   serverpage;
extern  struct  usermessage *rmessage;
extern  struct  windowdef   *wptr;


/* mouseorkey - wait for a mouse or key stroke */
/* int   mouseorkey()
   {  struct  mousemessage imsg;
   int     length;

   imsg.opcode   = fmouseorkey;
   imsg.pageno   = identifypage();
   getserverpage();
   sendmessage ( serverpage, sizeof ( struct mousemessage ), &imsg );
   length = getmessage ( rmessage );
   return rmessage->retstatus;
   }
*/

/* waitforevent - wait for any event */
int  waitforevent()
{  struct  mousemessage imsg;
   int     length;

   imsg.opcode   = fwaitforevent;
   imsg.pageno   = identifypage();
   getserverpage();
   sendmessage ( serverpage, sizeof ( struct mousemessage ), &imsg );
   length = getmessage ( rmessage );
   return rmessage->retstatus;
}

/* wakeup is a special function that just sends an event to the windows
   server to see if it is alive.  If so, it returns a -1 */
int  wakeup()
{  struct  mousemessage imsg;
   int     length;

   imsg.opcode   = fwakeup;
   imsg.pageno   = identifypage();
   serverpage    = findtask ( "WINAPI" );
   if ( serverpage == 0 ) {
     return 0;
   }
   sendmessage ( serverpage, sizeof ( struct mousemessage ), &imsg );
   length = getmessage ( rmessage );
   return rmessage->retstatus;
}


/* exitserver - exits the WINAPI server (not recommended) */
void  exitserver ()
{  struct  menumessage  imsg;

   imsg.opcode   = fexit;
   imsg.pageno   = identifypage();
   getserverpage();
   sendmessage ( serverpage, sizeof ( struct fontmessage ), &imsg );
   return;
}

