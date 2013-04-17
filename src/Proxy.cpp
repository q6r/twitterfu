#include "Proxy.h"

Proxy::Proxy(User *p) : parent(p) {
}

Proxy::~Proxy() {
}

void Proxy::setup() {
    // If we have proxy settings either from db or else.
    if(!Proxy::get("address").empty() && !Proxy::get("port").empty()) {
        parent->getTwitterObj().setProxyServerIp( Proxy::get("address") );
        parent->getTwitterObj().setProxyPassword( Proxy::get("port") );
        cout << "[+] Using proxy " << Proxy::get("address") << ":" << Proxy::get("port") << endl;
        if(!Proxy::get("username").empty() && !Proxy::get("password").empty()) {
                parent->getTwitterObj().setProxyUserName( Proxy::get("username") );
                parent->getTwitterObj().setProxyPassword( Proxy::get("password") );
        }
    }
}

void Proxy::set(const string & name, const string & data) {
    conf[name] = data;
}

string & Proxy::get(const string &name) {
    return conf[name];
}

bool Proxy::change_proxy(const string &address, const string &port,
	     const string &username, const string &password)
{

    Proxy::set("address", address);
    Proxy::set("port", port);
    Proxy::set("username", username);
    Proxy::set("password", password);
    this->setup();
    // sets parent getTwitterObj() to proxify

	return true;
}
