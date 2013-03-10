#ifndef DATABASE_H_
#define DATABASE_H_
#include "twitterfu.h"
#include "common.h"
#include <vector>

/**
 * Helps with dealing with sqlite3 database
 * @file database.h
 */

/**
 * This will remove duplicates from ToFollow
 * against the other tables
 * @param user the user object
 * @return true if we've removed duplicates
 */
bool removeDuplicatesInToFollow(User * user);
/**
 * Delete everything in a database
 * @param user the user object
 * @param table the table to delete
 * @return false if unable to delete
 */
bool purgeTable(User * user, std::string table);
/**
 * Reads a specific table and return the results in the value
 * as a std::vector of std::strings
 * @param user the user object
 * @param table the table name in the database
 * @param value the value to get from table
 * @return the result of select value from table in
 * as string vector
 */
std::vector < std::string > toVector(User * user, std::string table,
				     std::string value);

/**
 * Will std::cin some information
 * and add them to the database
 * @param user the user object
 * @return true if user created successfuly
 */
bool createUser(User * user);
/**
 * Get a value col from a table return std::vector.
 * @param user the user object
 * @param table the table in database
 * @param col the column in database
 * @return vector string of data in column in table in database
 */
std::vector < std::string > getVal(User * user, std::string table,
				   std::string col);
/**
 * It creates necessary tables if they don't exists
 * @param user the user object
 * @return false if unable to connect to database
 */
bool initalize(User * user);
/** 
 * Will insert or replace UNIQUE a std::vector<std::string>.
 * @param user the user object
 * @param v the vector to be inserted into the database
 * @param table the table to be used
 * @param values the values /col
 * @return false if unable to insert
 */
bool
toDB(User * user, std::vector < std::string > v, std::string table,
     std::string values);
/**
 * Check if there's a user in the DB.
 * @param user the user object
 * @return true if user exists
 */
bool userExist(User * user);
#endif
