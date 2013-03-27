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
    void set(string name, string & data);
    string & get(string name);
    /**
     * Change proxy and set it in database
     * @param user the user object
     * @param address ip address of the proxy
     * @param port port number of the proxy
     * @param username username of proxy
     * @param password password of proxy
     * @return true if successfuly changed proxy
     */
    bool change_proxy(string address, string port, string username, string password);
     /**
      * Check if the parent have set the values of the proxy
      * if so then setup the parent's twitterObj to use that proxy
      */
     void setup();
private:
     /* Takes conf[name] = data; */
     unordered_map<std::string, std::string> conf;
     User *parent;
};
#endif
