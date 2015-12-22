#include "common.h"
#include <math.h>
#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <event2/listener.h>
using namespace std;

struct Context
{
	int count;
	int64_t bytes;
	SessionMessage session;
	std::vector<char> buffer;
	event_base *base;
	Context()
		: count(0),
		bytes(0)
	{
		session.number = 0;
		session.length = 0;
	}
};


namespace transmit
{
	void readcallback(bufferevent *bev, void *arg)
	{
		Context *context = static_cast<Context*>(arg);
		int32_t len;
		bufferevent_read(bev, &len, sizeof(len));
		if (ntohl(len) == ntohl(context->session.length) && context->count < ntohl(context->session.number))
		{
			bufferevent_write(bev, &context->buffer, context->buffer.size());
			++context->count;
		}
		else
		{
			bufferevent_free(bev);
		}
	}
	void transmit(Options &options)
	{
		timeval start, end;
		Context *context = new Context();
		struct event_base *base = event_init();
		struct bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(options.port);
		inet_aton(options.host.c_str(), &server_addr.sin_addr);
		if (bufferevent_socket_connect(bev, static_cast<struct sockaddr*>(reinterpret_cast<void*>(&server_addr)), sizeof(server_addr)))
		{
			perror("connect failed\n");
			exit(-1);
		}
		context->session.length = htonl(options.length);
		context->session.number = htonl(options.number);
		context->buffer.resize(options.length);
		for (int i = 0; i < options.length; ++i)
		{
			context->buffer[i] = "012345689ABCDEF"[i % 16];
		}
		bufferevent_setcb(bev, readcallback, NULL, NULL, static_cast<void*>(context));
		bufferevent_enable(bev, EV_READ | EV_PERSIST);
		gettimeofday(&start, NULL);
		bufferevent_write(bev, static_cast<const void*>(&context->session), sizeof(context->session));
		event_base_dispatch(base);
		gettimeofday(&end, NULL);
		double diff = end.tv_sec - start.tv_sec + fabs(end.tv_usec - start.tv_usec) / 1000000;
		double total = 1.0 * options.length * options.number / 1024 / 1024;
		printf("%0.3fMiB transferred\n%0.3fMiB/s\n", total, total / diff);
	}
}
namespace receive
{
	void readcallback(bufferevent *bev, void *arg)
	{
		Context *context = static_cast<Context*>(arg);
		if ( context->session.number == 0)
		{
			SessionMessage session;
			bufferevent_read(bev, &session, sizeof(session));
			context->session.length = ntohl(session.length);
			context->session.number = ntohl(session.number);
			context->buffer.resize(context->session.length);
			bufferevent_write(bev, static_cast<const void*>(&session.length), sizeof(session.length));
			printf("length: %d, number: %d\n", ntohl(session.length), ntohl(session.number));
		}
		else
		{
			if (evbuffer_get_length(bufferevent_get_input(bev)) >= context->session.length)
			{
				bufferevent_read(bev, const_cast<void*>(reinterpret_cast<const void*>(context->buffer.data())), context->buffer.size());
				int32_t len = htonl(context->session.length);
				bufferevent_write(bev, static_cast<const void*>(&len), sizeof(len));
			}
		}
	}
	void eventcallback(bufferevent *bev, short what, void *arg)
	{
		Context *context = static_cast<Context*>(arg);
		delete context;
	}
	void listenercallback(struct evconnlistener* listener, evutil_socket_t  fd, struct sockaddr *addr, int socklen, void *arg)
	{
		struct event_base *base = (struct event_base *)arg;
		Context *context = new Context();
		context->base = base;
		struct bufferevent *buffer = bufferevent_socket_new(context->base, fd, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(buffer, readcallback, NULL, eventcallback, static_cast<void*>(context));
		bufferevent_enable(buffer, EV_READ | EV_PERSIST);
	}
	void receive(Options &options)
	{
		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(options.port);
		struct event_base *base = event_init();
		evconnlistener *listener = evconnlistener_new_bind(base, listenercallback, static_cast<void*>(base),
			LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, static_cast<struct sockaddr*>(reinterpret_cast<void*>(&server_addr)), sizeof(server_addr));
		if (!listener)
		{
			perror("create listener failed");
			exit(-1);
		}
		event_base_dispatch(base);
	}

}

int main(int argc, char *argv[])
{
	Options options;
	parseCommandLine(argc, argv, options);
	if (options.transmit)
	{
		transmit::transmit(options);
	}
	else if (options.receive)
	{
		receive::receive(options);
	}
	return 0;
}
