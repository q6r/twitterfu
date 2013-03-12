#ifndef PROXY_H_
#define PROXY_H_
#include <iostream>
#include <string>
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
                void setAddress(string n);
                void setPort(string n);
                void setUsername(string n);
                void setPassword(string n);
                string & getAddress();
                string & getPort();
                string & getUsername();
                string & getPassword();
                /**
                 * Change proxy and set it in database
                 * @param user the user object
                 * @param address ip address of the proxy
                 * @param port port number of the proxy
                 * @param username username of proxy
                 * @param password password of proxy
                 * @return true if successfuly changed proxy
                 */
                bool change_proxy(string address, string port,
                                  string username, string password);
        private:
                /** ip address of proxy */
                string address;
                /** port number of proxy */
                string port;
                /** username of proxy */
                string username;
                /** password of proxy */
                string password;
                /** the User aka parent */
                User *parent;
};
#endif
