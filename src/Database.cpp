#include "Database.h"

Database::Database(User *p) : parent(p) {
        string query;

	// Connect to database and create tables if
	// needed
	if (parent->db.connect(parent->getDBname().c_str()) == 0) {
                // Create necessary tables
                parent->db.execute
                    ("CREATE TABLE MyFollowers(Id integer PRIMARY KEY,userid text UNIQUE);");
                parent->db.execute
                    ("CREATE TABLE ToFollow(Id integer PRIMARY KEY,userid text UNIQUE);");
                parent->db.execute
                    ("CREATE TABLE Followed(Id integer PRIMARY KEY,userid text UNIQUE);");
                parent->db.execute
                    ("CREATE TABLE UnFollowed(Id integer PRIMARY KEY,userid text UNIQUE);");
                parent->db.execute
                    ("CREATE TABLE Config(Id integer PRIMARY KEY, username text, password text, access_key text, access_secret text, proxy_username text, proxy_password text, proxy_address text, proxy_port text, timezone text);");
                parent->db.disconnect();
        }

        // If database doesn't have a user then create it
        // otherwise just set the parent according to what we have
        // in the database
        if(Database::userExist() == false) {
                Database::createUser();
        } else {
                parent->setUsername(Database::getVal( "Config", "username").at(0));
		parent->setAccessTokenKey(Database::getVal( "Config", "access_key").at(0));
		parent->setAccessTokenSecret(Database::getVal( "Config", "access_secret").at(0));
		parent->setTimezone(Database::getVal( "Config", "timezone").at(0));
                parent->proxy->setAddress( Database::getVal( "Config", "proxy_address").at(0));
                parent->proxy->setPort(Database::getVal( "Config", "proxy_port").at(0));
                parent->proxy->setUsername(Database::getVal( "Config", "proxy_username").at(0));
                parent->proxy->setPassword(Database::getVal( "Config", "proxy_password").at(0));
       }
}

Database::~Database() { 

}

vector < string > Database::toVector(string table,
				     string value)
{
	vector < string > results;
	string userid;
	sqlite3pp::query::iterator it;
	string q = "SELECT " + value + " FROM " + table + ";";

        parent->db.connect(parent->getDBname().c_str());
        sqlite3pp::query qry(parent->db, q.c_str());

	for (it = qry.begin(); it != qry.end(); ++it) {
		(*it).getter() >> userid;
		results.push_back(userid);
	}

        parent->db.disconnect();

	return results;
}

bool Database::setupTimezone(string n) {
        string query;

        parent->db.connect(parent->getDBname().c_str());

        query = "UPDATE Config SET timezone = \"" + n + "\";";

        // unable to setup timezone
        if (parent->db.execute(query.c_str()) != 0) {
                return false;
        }
        parent->db.disconnect();
        cout << "We have set the timezone to " << n << endl;
        return true;

}

bool Database::userExist()
{
        parent->db.connect(parent->getDBname().c_str());
	sqlite3pp::query qry(parent->db, "SELECT * from Config;");

	if (qry.begin() == qry.end())
		return false;

	parent->db.disconnect();
	return true;
}

bool Database::purgeTable(string table)
{
	string q;

	//parent->db.connect(parent->db_name.c_str());
        parent->db.connect(parent->getDBname().c_str());
	q = "DELETE FROM " + table + ";";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	parent->db.disconnect();
	return true;
}

bool Database::removeDuplicatesInToFollow()
{
	vector < string > v_tofollow(Database::toVector("ToFollow", "userid"));
	vector < string > v_followed(Database::toVector("Followed", "userid"));
	vector < string > v_unfollowed(Database::toVector("UnFollowed", "userid"));
	vector < string > v_myfollowers(Database::toVector("MyFollowers", "userid"));
	vector < string >::iterator it;

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
	if (parent->db.connect(parent->getDBname().c_str()) == 1)
		return false;

	if (parent->db.execute("DELETE FROM ToFollow;") == 1)
		return false;

	if (Database::toDB( v_tofollow, "ToFollow", "userid") == false)
		return false;

	parent->db.disconnect();

	return true;
}

vector < string > Database::getVal(string table,
				   string col)
{
	string val, q;
	vector < string > vals;
	sqlite3pp::query::iterator it;

	q = "SELECT " + col + " FROM " + table + ";";
	parent->db.connect(parent->getDBname().c_str());

	sqlite3pp::query qry(parent->db, q.c_str());

	for (it = qry.begin(); it != qry.end(); it++) {
		(*it).getter() >> val;
		vals.push_back(val);
	}

	parent->db.disconnect();

	return vals;
}

bool Database::createUser()
{
	string q;
        string temp;
	cout << "Creating a user" << endl;

	cout << "username : ";
	cin >> temp;
        parent->setUsername(temp);
	cout << "password : ";
	cin >> temp;
        parent->setPassword(temp);

	// Do we want to use proxies ?
	cout << "Do you want to use a proxy [y/n] ? ";
	cin >> q;
	if (q == "y" || q == "Y") {
		cout << "Proxy address  : ";
		//cin >> parent->proxy->address;
		cin >> temp;
                parent->proxy->setAddress(temp);
		cout << "Proxy port     : ";
                cin >> temp;
                parent->proxy->setPort(temp);
		
		    cout <<
		    "Do you want to use a proxy username, password [y/n] ? ";
		cin >> q;
		if (q == "y" || q == "Y") {
			cout << "Proxy username : ";
                        cin >> temp;
                        parent->proxy->setUsername(temp);
			cout << "Proxy password : ";
			cin >> temp;
                        parent->proxy->setPassword(temp);
		}
	}
	// Create inital user row
	if (parent->db.execute
	    ("INSERT INTO Config VALUES(1, \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\");")
	    != 0)
		return false;

	// update values in Config table
	q = "UPDATE Config SET username = \"" + parent->getUsername() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET password = \"" + parent->getPassword() +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;
	if (parent->proxy->getAddress() != "" && parent->proxy->getPort() != "") {
		if (parent->proxy->change_proxy
		    (parent->proxy->getAddress(), parent->proxy->getPort(),
		     parent->proxy->getUsername(), parent->proxy->getPassword()) == false) {
			cerr << "[-] Error : Unable to set proxy" << 
			    endl;
			return false;
		}

	}

	return true;
}

bool
Database::toDB(vector < string > v,
     string table, string values)
{
	string query;
	vector < string >::iterator it;

	// chose database
	if (parent->db.connect(parent->getDBname().c_str()) == 1)
		return false;

	// Inserting into the database
	if (parent->db.execute("BEGIN") == 1)
		return false;

	for (it = v.begin(); it != v.end(); it++) {
		query =
		    "INSERT OR REPLACE INTO " + table + " (" + values +
		    ") VALUES ('" + *it + "');";
		if (parent->db.execute(query.c_str()) == 1) {
			cerr << "[-] Error : toDB " << query << endl;
			return false;
		}
	}

	if (parent->db.execute("COMMIT") == 1)
		return false;

	parent->db.disconnect();
	return true;
}
