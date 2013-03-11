#ifndef PROXY_H_
#define PROXY_H_
#include <iostream>
#include <string>

using namespace std;
/**
 * Represents the proxy of a user
 */
class Proxy {
        public:
                Proxy();
                ~Proxy();
                void setAddress(string n);
                void setPort(string n);
                void setUsername(string n);
                void setPassword(string n);
                string & getAddress();
                string & getPort();
                string & getUsername();
                string & getPassword();
        private:
                /** ip address of proxy */
                string address;
                /** port number of proxy */
                string port;
                /** username of proxy */
                string username;
                /** password of proxy */
                string password;
};
#endif
