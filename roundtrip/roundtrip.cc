#include <event.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <event2/listener.h>
struct Message
{
	int64_t request;
	int64_t response;
};

struct TimerArg
{
	struct timeval val;
	struct bufferevent *bev;
	struct event *ev;
};

static const size_t frameLen = sizeof(Message);
int64_t curTime()
{
	struct timeval val;
	gettimeofday(&val, NULL);
	return val.tv_sec * 1000000 + val.tv_usec;
	return 0;
}
void readcallback(bufferevent *bev, void *arg)
{
	if (evbuffer_get_length(bufferevent_get_input(bev))
		>= frameLen)
	{
		Message message;
		bufferevent_read(bev, &message, sizeof(message));
		message.response = curTime();
		bufferevent_write(bev, &message, sizeof(message));
	}
}
void listenercallback(struct evconnlistener* listener, evutil_socket_t  fd, struct sockaddr *addr, int socklen, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	struct bufferevent *buffer = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(buffer, readcallback, NULL, NULL, NULL);
	bufferevent_enable(buffer, EV_READ | EV_PERSIST);
}
void server(int32_t port)
{
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	struct event_base *base = event_init();
	evconnlistener *listener = evconnlistener_new_bind(base, listenercallback, static_cast<void*>(base),
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, 
		static_cast<struct sockaddr*>(reinterpret_cast<void*>(&server_addr)), sizeof(server_addr));
	event_base_dispatch(base);
}
void clientreadcallback(struct bufferevent *bev, void *arg)
{
	if (evbuffer_get_length(bufferevent_get_input(bev))
		>= frameLen)
	{
		Message message;
		bufferevent_read(bev, &message, sizeof(message));
		int64_t send = message.request;
		int64_t their = message.response;
		int64_t back = curTime();
		int64_t mine = (send + back) / 2;
		printf("round trip %ld clock error %ld\n", back - send, their - mine);
	}
		
}
void timercallback(int fd, short event, void *args)
{
	TimerArg *timer = (TimerArg*)args;
	Message message;
	message.request = curTime();
	bufferevent_write(timer->bev, &message, sizeof(message));
	evtimer_add(timer->ev, &timer->val);
}

void client(const char* ip, int32_t port)
{
	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	inet_aton(ip, &client_addr.sin_addr);
	struct event_base *base = event_init();
	struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	if (bufferevent_socket_connect(bev, static_cast<struct sockaddr*>(reinterpret_cast<void*>(&client_addr)), sizeof(client_addr)))
	{
		perror("client error");
	}
	bufferevent_setcb(bev, clientreadcallback, NULL, NULL, NULL);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
	TimerArg arg;
	struct event *ev = evtimer_new(base, timercallback, (void*)&arg);
	arg.bev = bev;
	arg.ev = ev;
	arg.val.tv_sec = 0;
	arg.val.tv_usec = 2 * 100000;
	evtimer_add(ev, &arg.val);
	event_base_dispatch(base);
}
int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		server(atoi(argv[1]));
	}
	else if (argc == 3)
	{
		client(argv[1], atoi(argv[2]));
	}
	return 0;
}