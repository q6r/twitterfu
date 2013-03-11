#ifndef USER_H_
#define USER_H_

#include <iostream>
#include <string>
#include "sqlite3pp.h"
#include <twitcurl.h>
#include "filter.h"
#include "proxy.h"
#include "common.h"

using namespace std;

class User {
        public:
                User();
                ~User();
                void setUsername(string n);
                void setPassword(string n);
                void setConsumerKey(string n);
                void setConsumerSecret(string n);
                void setAccessTokenKey(string n);
                void setAccessTokenSecret(string n);
                void setDBname(string n);
                void setTimezone(string n);
                void setFollowers(string n);
                void setFollowing(string n);
                string & getUsername();
                string & getPassword();
                string & getConsumerKey();
                string & getConsumerSecret();
                string & getAccessTokenKey();
                string & getAccessTokenSecret();
                string & getDBname();
                string & getTimezone();
                string & getFollowers();
                string & getFollowing();

                sqlite3pp::database db;
                Proxy proxy;
                Filters filters;
                twitCurl twitterObj;
        private:
                string username;
                string password;
                string consumer_key;
                string consumer_secret;
                string access_token_key;
                string access_token_secret;
                string db_name;
                string timezone;
                string followers;
                string following;

};
#endif
