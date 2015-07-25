#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
void socket_read_cb(bufferevent *bev, void *arg){
	/*bufferevent_free(bev);*/
     char msg[4096] = { 0 };
     size_t len = bufferevent_read(bev, msg, sizeof(msg));
     bufferevent_write(bev, msg, strlen(msg));
}

void socket_event_cb(bufferevent *bev, short events, void *arg){
	struct sockaddr_in addr;
	socklen_t socklen = sizeof(addr);
	getpeername(bufferevent_getfd(bev), (struct sockaddr*)&addr, &socklen);
	printf("%s:%d event : %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), events);
	bufferevent_free(bev);
}

void listen_cb(struct evconnlistener *listen,
	evutil_socket_t fd, struct sockaddr *addr,
	int socklen, void *arg){
	printf("reveive new connection %s:%d\n",
		inet_ntoa(((struct sockaddr_in*)&addr)->sin_addr), ntohs(((struct sockaddr_in*)&addr)->sin_port));
	event_base *base = (event_base*)arg;
	bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, socket_read_cb, NULL, socket_event_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

int main(){
	event_base *base = event_base_new();
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	struct evconnlistener *listener =
		evconnlistener_new_bind(base, listen_cb, base, LEV_OPT_CLOSE_ON_EXEC
		| LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 1024,
		(const sockaddr*)&addr, sizeof(addr));
	event_base_dispatch(base);
	evconnlistener_free(listener);
	event_base_free(base);
	return 0;
}
