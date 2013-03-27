#include "Database.h"

Database::Database(User *p) : parent(p) {
    string query;

	// Connect to database and create tables if
	// needed
	if (parent->db.connect(parent->get("db_name").c_str()) == 0) {
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
        parent->set("username", Database::getVal( "Config", "username").at(0));
        parent->set("access_token_key", Database::getVal( "Config", "access_key").at(0));
        parent->set("access_token_secret", Database::getVal( "Config", "access_secret").at(0));
        parent->set("timezone", Database::getVal( "Config", "timezone").at(0));
        parent->proxy->set("address", Database::getVal( "Config", "proxy_address").at(0));
        parent->proxy->set("port", Database::getVal( "Config", "proxy_port").at(0));
        parent->proxy->set("username", Database::getVal( "Config", "proxy_username").at(0));
        parent->proxy->set("password", Database::getVal( "Config", "proxy_password").at(0));
   }
}

Database::~Database() { 

}

deque < string > Database::toVector(string table,
				     string value)
{
	deque < string > results;
	string userid;
	sqlite3pp::query::iterator it;
	string q = "SELECT " + value + " FROM " + table + ";";

    parent->db.connect(parent->get("db_name").c_str());
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

        parent->db.connect(parent->get("db_name").c_str());

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
        parent->db.connect(parent->get("db_name").c_str());
	sqlite3pp::query qry(parent->db, "SELECT * from Config;");

	if (qry.begin() == qry.end())
		return false;

	parent->db.disconnect();
	return true;
}

bool Database::purgeTable(string table)
{
	string q;

    parent->db.connect(parent->get("db_name").c_str());
	q = "DELETE FROM " + table + ";";
	if (parent->db.execute(q.c_str()) != 0)
		return false;

	parent->db.disconnect();
	return true;
}

bool Database::removeDuplicatesInToFollow()
{
	deque < string > v_tofollow(Database::toVector("ToFollow", "userid"));
	deque < string > v_followed(Database::toVector("Followed", "userid"));
	deque < string > v_unfollowed(Database::toVector("UnFollowed", "userid"));
	deque < string > v_myfollowers(Database::toVector("MyFollowers", "userid"));
	deque < string >::iterator it;

	// remove anything in myfollowers from tofollow list
    v_myfollowers.erase(remove_if(v_myfollowers.begin(), v_myfollowers.end(), [&v_tofollow](string x) {
                return (find(v_tofollow.begin(),v_tofollow.end(), x)!=v_tofollow.end()) ? true : false;
    }), v_myfollowers.end());

	// remove anything in followed from tofollow list
    v_followed.erase(remove_if(v_followed.begin(), v_followed.end(), [&v_tofollow](string x) {
                return (find(v_tofollow.begin(),v_tofollow.end(), x)!=v_tofollow.end()) ? true : false;
    }), v_followed.end());

	// remove anything in unfollowed from tofollow list
    v_unfollowed.erase(remove_if(v_unfollowed.begin(), v_unfollowed.end(), [&v_tofollow](string x) {
                return (find(v_tofollow.begin(),v_tofollow.end(), x)!=v_tofollow.end()) ? true : false;
    }), v_unfollowed.end());

	// Write the new tofollow to ToFollow table
	if (parent->db.connect(parent->get("db_name").c_str()) == 1)
		return false;

	if (parent->db.execute("DELETE FROM ToFollow;") == 1)
		return false;

	if (Database::toDB( v_tofollow, "ToFollow", "userid") == false)
		return false;

	parent->db.disconnect();

	return true;
}

deque < string > Database::getVal(string table,
				   string col)
{
	string val, q;
	deque < string > vals;
	sqlite3pp::query::iterator it;

	q = "SELECT " + col + " FROM " + table + ";";
	parent->db.connect(parent->get("db_name").c_str());

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
    parent->set("username", temp);
	cout << "password : ";
	cin >> temp;
    parent->set("password", temp);

	// Do we want to use proxies ?
	cout << "Do you want to use a proxy [y/n] ? ";
	cin >> q;
	if (q == "y" || q == "Y") {
		cout << "Proxy address  : ";
		//cin >> parent->proxy->address;
		cin >> temp;
        parent->proxy->set("address",temp);
		cout << "Proxy port     : ";
        cin >> temp;
        parent->proxy->set("port",temp);
		
        cout << "Do you want to use a proxy username, password [y/n] ? ";
		cin >> q;
		if (q == "y" || q == "Y") {
			cout << "Proxy username : ";
            cin >> temp;
            parent->proxy->set("username",temp);
			cout << "Proxy password : ";
			cin >> temp;
            parent->proxy->set("password",temp);
		}
	}
	// Create inital user row
	if (parent->db.execute
	    ("INSERT INTO Config VALUES(1, \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\");")
	    != 0)
		return false;

	// update values in Config table
	q = "UPDATE Config SET username = \"" + parent->get("username") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET password = \"" + parent->get("password") +
	    "\" WHERE Id=1;";
	if (parent->db.execute(q.c_str()) != 0)
		return false;
	if (parent->proxy->get("address") != "" && parent->proxy->get("port") != "") {
		if (parent->proxy->change_proxy
		    (parent->proxy->get("address"), parent->proxy->get("port"),
		     parent->proxy->get("username"), parent->proxy->get("password")) == false) {
			cerr << "[-] Error : Unable to set proxy" << endl;
			return false;
		}

	}

	return true;
}

bool Database::toDB(deque < string > v, string table, string values)
{
	string query;
	deque < string >::iterator it;

	// chose database
	if (parent->db.connect(parent->get("db_name").c_str()) == 1)
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
