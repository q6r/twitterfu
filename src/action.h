#ifndef ACTION_H_
#define ACTION_H_
#include "twitterfu.h"
#include "filter.h"
#include "database.h"
#include "common.h"

/**
 * Actions related to the user objcet and API
 * @file action.h
 */

/**
 * Get the limit
 * @param user the user object
 * @return number of remaining hits
 */
int getRemainingHits(User * user);
/**
 * Parse the last web response and get a
 * value of a node, save the value in T v
 * @param user the user object
 * @param node the key to get
 * @param v the values of the key
 * @return true of can read last response
 */
bool lastResponse(User * user, std::string node, std::string & v);
/**
 * Handler for follow/unfollow to set a exitFlag
 * @return n the catched signal number
 */
void signalHandler(int n);

/** 
 * It will follow a std::vector of user ids
 * @param to_follow the users to follow vector
 * @param user the user object
 */
void follow(std::vector < std::string > to_follow, User * user);
/**
 * Unfollow users who haven't followed me back
 * @param user the user object
 */
void unfollow(User * user);

/**
 * Get the followers of a user and create a std::vector
 * of their userIDs and return that std::vector
 * @param user the user object
 * @param username the username to get his followers
 * @return username followers as vector
 */

std::vector < std::string > getFollowers(User * user, std::string username);
/**
 * Fetch the following of a user and create a std::vector
 * of their userids and return that std::vector
 * @param user the user object
 * @param username the username to get his following
 * @return vector of username following
 */

std::vector < std::string > getFollowing(User * user, std::string username);
/**
 * Will take a query and return a std::vector of
 * user ids to be added to the to follow list. This function
 * is limited to 15 results since pages are not supported
 * by twitCurl.
 * @param user the user object
 * @param what the query to search for
 * @return vector of results
 */
std::vector < std::string > search(User * user, std::string what);
/**
 * Show database, account and API status.
 * @param user the user object
 * @return true of can read status
 */
bool status(User * user);
#endif
