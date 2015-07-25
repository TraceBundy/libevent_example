//作为分发消息
#ifndef HUB_H
#define HUB_H

#include <event2/event.h>
#include <event2/listener.h>
#include <map>
#include <event2/bufferevent.h>
#include <list>
#include <string>
#include <assert.h>

class Topic {
public:
	explicit Topic(std::string topic)
	:topic_(topic){}
	void pushMessage(std::string &content);
	void add(struct bufferevent *bev);
	void remove(struct bufferevent *bev);
private:
	std::string topic_;
	std::string content_;
	std::list<struct bufferevent *> subList_;
};
class Hub;
typedef struct CallbackArg{
	event_base *base;
	Hub *hub;
}CallbackArg;

class Hub {
public:
	explicit Hub(event_base *base, struct evconnlistener *listen)
		:base_(base), listener_(listen)
	{
		args_.base = base_;
		args_.hub = this;
	}
	void start(){
		evconnlistener_set_cb(listener_, onConnection, &args_);
		evconnlistener_enable(listener_);
	}
	static void onConnection(
		struct evconnlistener *listen, evutil_socket_t fd,
		struct sockaddr *addr, int socklen, void *arg);
	static void onMessageCallback(struct bufferevent *bev, void *arg);
	static void onEventCallback(struct bufferevent *bev, short what, void *arg);
private:
	typedef std::map<std::string, Topic> SubList;
	void addSub(struct bufferevent *bev, std::string &topic);
	void removeSub(struct bufferevent *bev, std::string &topic);
	void addPub(struct bufferevent *bev, std::string &topic, std::string &content);
	event_base *base_;
	CallbackArg args_;
	struct evconnlistener *listener_;
	std::map<std::string, Topic> subList_;
};

#endif