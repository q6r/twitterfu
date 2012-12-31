#include "twitterfu.h"

/* @method      : database::toVector
 * @description : Reads a specific table and return the results in the value
 * as a std::vector of std::strings
 * @input       : user, table, value
 * @output      : std::vector<std::string> of select value from table;
 */
std::vector < std::string > database::toVector(User * user, std::string table, std::string value)
{
	std::vector < std::string > results;
	std::string userid;
        sqlite3pp::query::iterator it;
	std::string q = "SELECT " + value + " FROM " + table + ";";

	user->db.connect(user->db_name.c_str());
	sqlite3pp::query qry(user->db, q.c_str());

	for (it = qry.begin(); it != qry.end(); ++it) {
		(*it).getter() >> userid;
		results.push_back(userid);
	}

	user->db.disconnect();

	return results;
}

/* @method      : database::userExist
 * @description : Check if there's a user in the DB.
 * @input       : User
 * @output      : true if exists otherwise false
 */
bool database::userExist(User * user)
{

	user->db.connect(user->db_name.c_str());
	sqlite3pp::query qry(user->db, "SELECT * from Config;");

	if (qry.begin() == qry.end())
		return false;

	user->db.disconnect();
	return true;
}

/*
 * @method      : database::purgeTable
 * @description : delete everything in a database
 * @input       : user, table
 * @outpt       : false if failed, otherwise true.
 */
bool database::purgeTable(User * user, std::string table)
{
	std::string q;

	user->db.connect(user->db_name.c_str());

	q = "DELETE FROM " + table + ";";
	if (user->db.execute(q.c_str()) != 0)
		return false;

	user->db.disconnect();
	return true;
}

/*
 * @method      : database::removeDuplicatesInToFollow
 * @descrption  : This will remove duplicates from ToFollow
 * against the other tables
 * @input       : user 
 * @output      : true if duplicates are removed, otherwise false
 */
bool database::removeDuplicatesInToFollow(User * user)
{
	std::vector < std::string > v_tofollow(database::toVector(user, "ToFollow", "userid"));
	std::vector < std::string > v_followed(database::toVector(user, "Followed", "userid"));
	std::vector < std::string >
	    v_unfollowed(database::toVector(user, "UnFollowed", "userid"));
	std::vector < std::string >
	    v_myfollowers(database::toVector(user, "MyFollowers", "userid"));
	std::vector < std::string >::iterator it;

	// remove anything in myfollowers from tofollow list
	for (it = v_myfollowers.begin(); it != v_myfollowers.end(); it++) {
		v_tofollow.erase(remove
				 (v_tofollow.begin(), v_tofollow.end(), *it),
				 v_tofollow.end());
	}

	// remove anything in followed from tofollow list
	for (it = v_followed.begin(); it != v_followed.end(); it++) {
		v_tofollow.erase(remove
				 (v_tofollow.begin(), v_tofollow.end(), *it),
				 v_tofollow.end());
	}

	// remove anything in unfollowed from tofollow list
	for (it = v_unfollowed.begin(); it != v_unfollowed.end(); it++) {
		v_tofollow.erase(remove
				 (v_tofollow.begin(), v_tofollow.end(), *it),
				 v_tofollow.end());
	}

	// Write the new tofollow to ToFollow table
	if (user->db.connect(user->db_name.c_str()) == 1)
		return false;

	if (user->db.execute("DELETE FROM ToFollow;") == 1)
		return false;

	if (database::toDB(user, v_tofollow, "ToFollow", "userid") == false)
		return false;

	user->db.disconnect();

	return true;
}

/*
 * @method      : database::getVal
 * @description : Get a value col from a table return std::vector.
 * @input       : user, table, col
 * @output      : std::vector of std::string
 */
std::vector < std::string > database::getVal(User * user, std::string table, std::string col)
{
	std::string val, q;
	std::vector < std::string > vals;
	sqlite3pp::query::iterator it;

	q = "SELECT " + col + " FROM " + table + ";";
	user->db.connect(user->db_name.c_str());

	sqlite3pp::query qry(user->db, q.c_str());

	for (it = qry.begin(); it != qry.end(); it++) {
		(*it).getter() >> val;
		vals.push_back(val);
	}

	user->db.disconnect();

	return vals;
}

/* @method      : database::createUser
 * @description : Will std::cin some information
 * and add them to the database
 * @input       : user
 * @output      : true if successfuly inserted user to database
 * otherwise false;
 */
bool database::createUser(User * user)
{
	std::string q;
	std::cout << "Creating a user" << std::endl;

	std::cout << "username : ";
	std::cin >> user->username;
	std::cout << "password : ";
	std::cin >> user->password;

	// Do we want to use proxies ?
	std::cout << "Do you want to use a proxy [y/n] ? ";
	std::cin >> q;
	if (q == "y" || q == "Y") {
		std::cout << "Proxy address  : ";
		std::cin >> user->proxy.address;
		std::cout << "Proxy port     : ";
		std::cin >> user->proxy.port;
		std::cout <<
		    "Do you want to use a proxy username, password [y/n] ? ";
		std::cin >> q;
		if (q == "y" || q == "Y") {
			std::cout << "Proxy username : ";
			std::cin >> user->proxy.username;
			std::cout << "Proxy password : ";
			std::cin >> user->proxy.password;
		}
	}
	// Create inital user row
	if (user->db.execute
	    ("INSERT INTO Config VALUES(1, \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\");")
	    != 0)
		return false;

	// update values in Config table
	q = "UPDATE Config SET username = \"" + user->username +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET password = \"" + user->password +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;
	if (user->proxy.address != "" && user->proxy.port != "") {
		if (change_proxy
		    (user, user->proxy.address, user->proxy.port,
		     user->proxy.username, user->proxy.password) == false) {
			std::cerr << "[-] Error : Unable to set proxy" << std::endl;
			return false;
		}

	}

	return true;
}

/*
 * @method      : database::initalize
 * @description : It creates necessary tables if they don't exists
 * @input       : User
 * @output      : false if unable to connect to db, true otherwise.
 */
bool database::initalize(User * user)
{
	std::string query;

	// Connect to database
	if (user->db.connect(user->db_name.c_str()) != 0)
		return false;

	// Create necessary tables
	user->db.execute
	    ("CREATE TABLE MyFollowers(Id integer PRIMARY KEY,userid text UNIQUE);");
	user->db.execute
	    ("CREATE TABLE ToFollow(Id integer PRIMARY KEY,userid text UNIQUE);");
	user->db.execute
	    ("CREATE TABLE Followed(Id integer PRIMARY KEY,userid text UNIQUE);");
	user->db.execute
	    ("CREATE TABLE UnFollowed(Id integer PRIMARY KEY,userid text UNIQUE);");
	user->db.execute
	    ("CREATE TABLE Config(Id integer PRIMARY KEY, username text, password text, access_key text, access_secret text, proxy_username text, proxy_password text, proxy_address text, proxy_port text, timezone text);");
	user->db.disconnect();

	return true;
}


/* @method      : toDB
 * @description : will insert or replace UNIQUE a std::vector<std::string>.
 * @input       : User, std::vector, table, values
 * @output      : true if successful false if unable to connect to db
 * or unable to insert to table.
 */
bool database::toDB(User * user, std::vector < std::string > v, std::string table, std::string values)
{
	std::string query;
	std::vector < std::string >::iterator it;

	// chose database
	if (user->db.connect(user->db_name.c_str()) == 1)
		return false;

	// Inserting into the database
	if (user->db.execute("BEGIN") == 1)
		return false;

	for (it = v.begin(); it != v.end(); it++) {
		query =
		    "INSERT OR REPLACE INTO " + table + " (" + values +
		    ") VALUES ('" + *it + "');";
		if (user->db.execute(query.c_str()) == 1) {
			std::cerr << "[-] Error : database::toDB " << query << std::endl;
			return false;
		}
	}

	if (user->db.execute("COMMIT") == 1)
		return false;

	user->db.disconnect();
	return true;
}

