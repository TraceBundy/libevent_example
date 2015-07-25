#include "Hub.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include "Codec.h"
using namespace std;
void Topic::pushMessage(string &content){
	content_ = content;
	for (list<struct bufferevent*>::iterator it = subList_.begin();
		it != subList_.end(); ++it){
		bufferevent_write(*it, content.c_str(), content.length());
	}
}

void Topic::add(struct bufferevent *bev){
	subList_.push_back(bev);
}

void Topic::remove(struct bufferevent *bev){
	for (list<struct bufferevent*>::iterator it = subList_.begin();
		it != subList_.end(); ++it){
		if (bev == *it) {
			subList_.erase(it);
			break;
		}
	}
}

void Hub::onConnection(
	struct evconnlistener *listen, evutil_socket_t fd,
	struct sockaddr *addr, int socklen, void *arg) {
		CallbackArg *call = (CallbackArg*)arg;
		assert(call->base != NULL);
		struct bufferevent *buffer = 
			bufferevent_socket_new(
			call->base, fd, BEV_OPT_CLOSE_ON_FREE);
		if (buffer){
			bufferevent_setcb(
				buffer, onMessageCallback, NULL, onEventCallback, call);
			bufferevent_enable(buffer, EV_READ | EV_PERSIST);
		} else {
			printf("create buffer failed\n");
		}
}

void Hub::onMessageCallback(struct bufferevent *bev, void *arg){
	CallbackArg *call = (CallbackArg*)arg;
	Hub *hub = call->hub;
	size_t len = 0;
	struct evbuffer *buffer = evbuffer_new();
	bufferevent_read_buffer(bev, buffer);
	char *msg = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
	if (len > 0){
		string topic, content;
		ParseResult res = parseMessage(msg, len, topic, content);
		switch (res){
		case SUB_CMD:
			printf("SUB_CMD %s\n", topic.c_str());
			hub->addSub(bev, topic);
			break;
		case UNSUB_CMD:
			printf("UNSUB_CMD %s\n", topic.c_str());
			hub->removeSub(bev, topic);
			break;
		case PUB_CMD:
			printf("PUB_CMD %s\n", topic.c_str());
			hub->addPub(bev, topic, content);
			break;
		default:
			break;
		}
	} else {
		bufferevent_disable(bev, EV_READ);
	}
}

void Hub::onEventCallback(struct bufferevent *bev, short what, void *arg){

}

void Hub::addSub(struct bufferevent *bev, std::string &topic){
	SubList::iterator it = subList_.find(topic);
	if (it == subList_.end()){
		it = subList_.insert(make_pair(topic, Topic(topic))).first;
		
	}
	it->second.add(bev);
}

void Hub::removeSub(struct bufferevent *bev, std::string &topic){
	SubList::iterator it = subList_.find(topic);
	if (it != subList_.end()){
		it->second.remove(bev);
	}
}

void Hub::addPub(
	struct bufferevent *bev, std::string &topic, std::string &content){
	SubList::iterator it = subList_.find(topic);
	if (it == subList_.end()){
		it = subList_.insert(make_pair(topic, Topic(topic))).first;
	}
	it->second.pushMessage(content);
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("./Hub port\n");
		return 0;
	}

	event_base *base = event_base_new();
	assert(base != NULL);
	struct sockaddr_in listenAddr;
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(atoi(argv[1]));
	struct evconnlistener *listen = evconnlistener_new_bind(
		base, NULL, base, LEV_OPT_CLOSE_ON_EXEC
		| LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
		1024, (struct sockaddr*)&listenAddr, sizeof(listenAddr));
	Hub hub(base, listen);
	hub.start();
	event_base_dispatch(base);
	return 0;
}