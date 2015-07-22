#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>

#define NEVENT 2
struct event *ev[NEVENT];

static void time_cb(evutil_socket_t fd, short event, void *arg){
    struct event *one = (struct event*) arg;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    evtimer_add(one, &tv);
    printf("-----------\n");
}
int main(){
    struct timeval tv;
    event_init();
    //struct event_base *base = event_base_new();
    for (int i = 0; i < NEVENT; i++){
        ev[i] = (struct event*)malloc(sizeof(struct event));
        //ev[i] = event_new(base, -1, 0, time_cb, NULL);
        evtimer_set(ev[i], time_cb, ev[i]);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        evtimer_add(ev[i], &tv);
        //event_add(ev[i], &tv);
    }
    //event_base_dispatch(base);
    event_dispatch();
    return 0;
}
