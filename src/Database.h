#ifndef DATABASE_H_
#define DATABASE_H_
#include <iostream>
#include <string>
#include <deque>
#include "User.h"

using namespace std;
class User;

class Database {
        public:
                /**
                 * Initalize the users's database
                 * create necessary tables
                 * @param p parent User
                 */ 
                Database(User *p);
                ~Database();
                /**
                 * This will remove duplicates from ToFollow
                 * against the other tables
                 * @param user the user object
                 * @return true if we've removed duplicates
                 */
                bool removeDuplicatesInToFollow();
                /**
                 * Delete everything in a database
                 * @param user the user object
                 * @param table the table to delete
                 * @return false if unable to delete
                 */
                bool purgeTable(string table);
                /**
                 * Reads a specific table and return the results in the value
                 * as a deque of strings
                 * @param user the user object
                 * @param table the table name in the database
                 * @param value the value to get from table
                 * @return the result of select value from table in
                 * as string deque
                 */
                deque < string > toVector(string table, string value);
                /**
                 * Setup timezone in database
                 * @param n the timezone
                 */ 
                bool setupTimezone(string n);
                /**
                 * TODO might move to User. 
                 * Will cin some information
                 * and add them to the database
                 * @param user the user object
                 * @return true if user created successfuly
                 */
                bool createUser();
                /**
                 * Get a value col from a table return deque.
                 * @param user the user object
                 * @param table the table in database
                 * @param col the column in database
                 * @return deque string of data in column in table in database
                 */
                deque < string > getVal(string table,
                                                   string col);
                /** 
                 * Will insert or replace UNIQUE a deque<string>.
                 * @param user the user object
                 * @param v the deque to be inserted into the database
                 * @param table the table to be used
                 * @param values the values /col
                 * @return false if unable to insert
                 */
                bool
                toDB(deque < string > v, string table,
                     string values);
                /**
                 * Check if there's a user in the DB.
                 * @param user the user object
                 * @return true if user exists
                 */
                bool userExist();
        private:
                User *parent;
};
#endif
