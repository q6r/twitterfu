#include "Proxy.h"

Proxy::Proxy(User *p) : parent(p) {
}

Proxy::~Proxy() {
}

void Proxy::setup() {
    // If we have proxy settings either from db or else.
    if(!Proxy::get("address").empty() && !Proxy::get("port").empty()) {
        parent->twitterObj.setProxyServerIp( Proxy::get("address") );
        parent->twitterObj.setProxyPassword( Proxy::get("port") );
        cout << "[+] Using proxy " << Proxy::get("address") << ":" << Proxy::get("port") << endl;
        if(!Proxy::get("username").empty() && !Proxy::get("password").empty()) {
                parent->twitterObj.setProxyUserName( Proxy::get("username") );
                parent->twitterObj.setProxyPassword( Proxy::get("password") );
        }
    }
}

void Proxy::set(string name, string & data) {
    conf[name] = data;
}

string & Proxy::get(string name) {
    return conf[name];
}

bool Proxy::change_proxy(string address, string port,
	     string username, string password)
{

	string q;

    Proxy::set("address", address);
    Proxy::set("port", port);
    Proxy::set("username", username);
    Proxy::set("password", password);

	parent->db.connect(parent->getDBname().c_str());

	// update DB with new proxy
	q = "UPDATE Config SET proxy_address = \"" + Proxy::get("address") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_port = \"" + Proxy::get("port") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_username = \"" + Proxy::get("username") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_password = \"" + Proxy::get("password") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	parent->db.disconnect();
	return true;
}
