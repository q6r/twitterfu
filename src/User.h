#ifndef USER_H_
#define USER_H_

#include <iostream>
#include <string>
#include <signal.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <twitcurl.h>
#include <unordered_map>
#include "Proxy.h"
#include "Filters.h"

using namespace std;
class Filters;
class Proxy;
class Database;

class User {
public:
    /**
     * Construct the user and initalize necessary things.
     * create filters,
     * create proxy, and set it up.
     * @param _consumer_key the consumer key
     * @param _consumer_secret the consumer secret
     */ 
    User(string _username, string _password, string _consumer_key, string _consumer_secret);
    ~User();
    void set(string key, string val);
    string & get(string key);
    /**
     * Show database, account and API status.
     * @param user the user object
     * @return true of can read status
     */
    bool status();
    /**
     *
     * Reached limit ? true reached, false
     * haven't reached.
     */
    bool reachedLimit();
    /**
     * Return the authenticated flag
     * true if authenticated, false if failed
     * to authenticated.
     */
    bool isAuthenticated();
    /**
     * Unfollow users who haven't followed me back
     * @param user the user object
     */
    void unfollow();
    /**
     *
     *
     */
    bool unfollow(string id);
    /** 
     * It will follow a deque of user ids
     * @param to_follow the users to follow deque
     * @param user the user object
     */
    void follow(deque < string > to_follow);
    /**
     *
     *
     */
    bool follow(string id);
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
     * signal flag for signal handling
     */  
    static bool gotExitSignal;
    friend Proxy;   // access to twitterObj
    friend Filters; // access to twitterObj 

    // self following and followers
    deque< string > my_followers;
    deque< string > my_following;
private:
    twitCurl twitterObj; /** twitterObject of the user */
    /*
     * keys:
     *  username,
     *  password,
     *  consumer_key,
     *  consumer_secret,
     *  access_token_key,
     *  access_token_secret,
     *  db_name,
     *  timezone,
     *  followers,
     *  following
     */
    unordered_map<string, string> conf;
    /**
     * return a random number between from and to
     * @param from the from number
     * @param to the to number
     * @return number between [from,two]
     */
    int randomize(int from, int to);
    /**
     * Back to the first line and erase n characters
     * @param n number of blanks
     */
    void cleanLine(int n);
    /**
     * Authenticate the user
     */
    bool authenticate();
    bool authenticated;   /* Is authenticated */ 
    /**
     * Get the limit
     * @param user the user object
     * @return number of remaining hits
     */
    int getRemainingHits();
};
#endif
