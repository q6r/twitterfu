#ifndef USER_H_
#define USER_H_

#include <iostream>
#include <string>
#include <twitcurl.h>
#include "sqlite3pp.h"
#include "Proxy.h"
#include "Filters.h"
#include "twitterfu.h"

using namespace std;
class Filters;
class Proxy;

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
                /**
                 * Authenticate the user
                 * @param user the user object
                 * @return true of user authenticated successfuly
                 */
                bool authenticate();
                /**
                 * Get the limit
                 * @param user the user object
                 * @return number of remaining hits
                 */
                int getRemainingHits();
                /**
                 * Show database, account and API status.
                 * @param user the user object
                 * @return true of can read status
                 */
                bool status();
                /**
                 * Unfollow users who haven't followed me back
                 * @param user the user object
                 */
                void unfollow();
                /**
                 * Configure various options for user
                 * @param user the user object
                 * @return true if configured successfuly
                 */
                bool configure();
                /** 
                 * It will follow a vector of user ids
                 * @param to_follow the users to follow vector
                 * @param user the user object
                 */
                void follow(vector < string > to_follow);
                /**
                 * Get the followers of a user and create a vector
                 * of their userIDs and return that vector
                 * @param user the user object
                 * @param username the username to get his followers
                 * @return username followers as vector
                 */
                vector < string > getFollowers(string username);
                /**
                 * Fetch the following of a user and create a vector
                 * of their userids and return that vector
                 * @param user the user object
                 * @param username the username to get his following
                 * @return vector of username following
                 */
                vector < string > getFollowing(string username);
                /**
                 * Will take a query and return a vector of
                 * user ids to be added to the to follow list. This function
                 * is limited to 15 results since pages are not supported
                 * by twitCurl.
                 * @param user the user object
                 * @param what the query to search for
                 * @return vector of results
                 */
                vector < string > search(string what);
                /**
                 * Parse the last web response and get a
                 * value of a node, save the value in T v
                 * @param user the user object
                 * @param node the key to get
                 * @param v the values of the key
                 * @return true of can read last response
                 */
                bool lastResponse(string node, string & v);
                /**
                 * Handler for follow/unfollow to set a exitFlag
                 * @return n the catched signal number
                 */
                static void signalHandler(int n);
                /**
                 * Verify that the user authenticated
                 * successfuly.
                 * @return true or false
                 */
                bool verify();
                
                sqlite3pp::database db;
                Proxy *proxy;
                Filters *filters;
                static bool gotExitSignal;

                friend Proxy;   // access to twitterObj
                friend Filters; // access to twitterObj 
        private:
                twitCurl twitterObj;
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
