#ifndef PROXY_H_
#define PROXY_H_
#include <iostream>
#include <string>
#include <unordered_map>
#include "User.h"

using namespace std;
class User;

/**
 * Represents the proxy of a user
 */
class Proxy {
public:
    Proxy(User *p);
    ~Proxy();
    void set(const string &name, const string &data);
    string & get(const string &name);
    /**
     * Change proxy and set it in database
     * @param user the user object
     * @param address ip address of the proxy
     * @param port port number of the proxy
     * @param username username of proxy
     * @param password password of proxy
     * @return true if successfuly changed proxy
     */
    bool change_proxy(const string &address, const string &port, const string &username, const string &password);
    /**
     * apply proxy changes
     */
    void setup();
private:
     /* Takes conf[name] = data; */
     unordered_map<std::string, std::string> conf;
     User *parent;
};
#endif
