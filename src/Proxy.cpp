#include "Proxy.h"

Proxy::Proxy() {
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
