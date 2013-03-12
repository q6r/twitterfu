#include "Proxy.h"

Proxy::Proxy(User *p) : parent(p) {
}

Proxy::~Proxy() {
}

void Proxy::setAddress(string n) {
        address = n;
}

void Proxy::setPort(string n) {
        port = n;
}

void Proxy::setUsername(string n) {
        username = n;
}

void Proxy::setPassword(string n) {
        password = n;
}

string & Proxy::getAddress() {
        return address;
}

string & Proxy::getPort() {
        return port;
}

string & Proxy::getUsername() {
        return username;
}

string & Proxy::getPassword() {
        return password;
}

bool Proxy::change_proxy(string address, string port,
	     string username, string password)
{

	string q;

        Proxy::setAddress(address);
        Proxy::setPort(port);
        Proxy::setUsername(username);
        Proxy::setPassword(password);

	parent->db.connect(parent->getDBname().c_str());

	// update DB with new proxy
	q = "UPDATE Config SET proxy_address = \"" + Proxy::getAddress() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_port = \"" + Proxy::getPort() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_username = \"" + Proxy::getUsername() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	q = "UPDATE Config SET proxy_password = \"" + Proxy::getPassword() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	parent->db.disconnect();
	return true;
}
