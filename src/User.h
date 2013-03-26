#ifndef USER_H_
#define USER_H_

#include <iostream>
#include <string>
#include <twitcurl.h>
#include "sqlite3pp.h"
#include "Proxy.h"
#include "Filters.h"
#include "Database.h"
#include "twitterfu.h"

using namespace std;
class Filters;
class Proxy;
class Database;

class User {
        public:
                /**
                 * Construct the user and initalize necessary things.
                 * create database,
                 * create filters,
                 * create proxy, and set it up.
                 * @param _database the database name
                 * @param _consumer_key the consumer key
                 * @param _consumer_secret the consumer secret
                 */ 
                User(string _database, string _consumer_key, string _consumer_secret);
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
                string getUsername();
                string getPassword();
                string getConsumerKey();
                string getConsumerSecret();
                string getAccessTokenKey();
                string getAccessTokenSecret();
                string getDBname();
                string getTimezone();
                string getFollowers();
                string getFollowing();
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
                 * It will follow a deque of user ids
                 * @param to_follow the users to follow deque
                 * @param user the user object
                 */
                void follow(deque < string > to_follow);
                /**
                 * Get the followers of a user and create a deque
                 * of their userIDs and return that deque
                 * @param user the user object
                 * @param username the username to get his followers
                 * @return username followers as deque
                 */
                deque < string > getFollowers(string username);
                /**
                 * Fetch the following of a user and create a deque
                 * of their userids and return that deque
                 * @param user the user object
                 * @param username the username to get his following
                 * @return deque of username following
                 */
                deque < string > getFollowing(string username);
                /**
                 * Will take a query and return a deque of
                 * user ids to be added to the to follow list. This function
                 * is limited to 15 results since pages are not supported
                 * by twitCurl.
                 * @param user the user object
                 * @param what the query to search for
                 * @return deque of results
                 */
                deque < string > search(string what);
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
                /**
                 * The User's Proxy
                 */ 
                Proxy *proxy;
                /**
                 * The USers' Filters
                 */ 
                Filters *filters;
                /**
                 *
                 */
                Database *database; 
                /**
                 * signal flag for signal handling
                 */  
                static bool gotExitSignal;
                friend Proxy;   // access to twitterObj
                friend Filters; // access to twitterObj 
                friend Database; // access to db, twitterObj
        private:
                /**
                 * sqlite3 database
                 */ 
                sqlite3pp::database db;
                twitCurl twitterObj; /** twitterObject of the user */
                string username; /** username */
                string password; /** password */
                string consumer_key; /** consumer key */
                string consumer_secret; /** consumer secret */
                string access_token_key; /** access token key */
                string access_token_secret; /** access token secret */
                string db_name; /** database name used for sqlite3 */
                string timezone; /** timezone of the user */
                string followers; /** # of followers of the user */
                string following; /** # of following of the user */

};
#endif
