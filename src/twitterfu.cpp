#include "twitterfu.h"

/* Globals
 */
bool gotExitSignal = false;

using namespace std;
using namespace sqlite3pp;

using boost::property_tree::ptree;

/* @method      : search
 * @description : Will take a query and return a vector of
 * user ids to be added to the to follow list. This function
 * is limited to 15 results since pages are not supported
 * by twitCurl.
 * @input       : User, query
 * @output      : vector of user ids
 */
vector < string > search(User * user, string query)
{
	string jsonResult;
	ptree pt;
	vector < string > ids;

	// replace all spaces with %20
	for (size_t pos = query.find(' '); pos != string::npos;
	     pos = query.find(' ', pos))
		query.replace(pos, 1, "%20");

	// send the search query
	if (user->twitterObj.search(query) == false)
		return ids;

	// Get results, parse then and push to ids vector
	user->twitterObj.getLastWebResponse(jsonResult);
	stringstream ss(jsonResult);

	try {
		read_json(ss, pt);
		BOOST_FOREACH(const ptree::value_type & child,
			      pt.get_child("results")) {
			ids.push_back(child.second.get < string >
				      ("from_user_id"));
		}
	} catch(exception const &e) {
		cerr << "(Err:Unable to parse json)" << endl;
		return ids;
	}

	return ids;
}

/*
 * @method      : optionSelect
 * @output      : the options selected
 */
int optionSelect()
{
	int opt = -1;
	cout << "> ";
	cin >> opt;
	if (cin.fail() == true) {
		cin.clear();
		cin.ignore(numeric_limits < streamsize >::max(), 0x0a);
		return -1;
	}
	return opt;
}

/* @method      : createUser
 * @description : Will cin some information
 * and add them to the database
 * @input       : User struct
 * @output      : true if successfuly inserted user to database
 * otherwise false;
 */
bool createUser(User * user)
{
	string q;
	cout << "Creating a user" << endl;

	cout << "username : ";
	cin >> user->username;
	cout << "password : ";
	cin >> user->password;

	// Do we want to use proxies ?
	cout << "Do you want to use a proxy [y/n] ? ";
	cin >> q;
	if (q == "y" || q == "Y") {
		cout << "Proxy address  : ";
		cin >> user->proxy.address;
		cout << "Proxy port     : ";
		cin >> user->proxy.port;
		cout <<
		    "Do you want to use a proxy username, password [y/n] ? ";
		cin >> q;
		if (q == "y" || q == "Y") {
			cout << "Proxy username : ";
			cin >> user->proxy.username;
			cout << "Proxy password : ";
			cin >> user->proxy.password;
		}
	}
	// Create inital user row
	if (user->db.execute
	    ("INSERT INTO Config VALUES(1, \"\", \"\", \"\", \"\", \"\", \"\", \"\", \"\");")
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
			cerr << "[-] Error : Unable to set proxy" << endl;
			return false;
		}

	}

	return true;
}

/* @method      : userExistInDB
 * @description : Check if there's a user in the DB.
 * @inpt        : User
 * @output      : true if exists otherwise false
 */
bool userExistInDB(User * user)
{

	user->db.connect(user->db_name.c_str());
	query qry(user->db, "SELECT * from Config;");

	if (qry.begin() == qry.end())
		return false;

	user->db.disconnect();
	return true;
}

/* @method      : dbToVector
 * @description : Reads a specific table and return the results in the value
 * as a vector of strings
 * @input       : user, table, value
 * @output      : vector<string> of select value from table;
 */
vector < string > dbToVector(User * user, string table, string value)
{
	vector < string > results;
	string userid;
	string q = "SELECT " + value + " FROM " + table + ";";

	user->db.connect(user->db_name.c_str());
	query qry(user->db, q.c_str());

	for (query::iterator it = qry.begin(); it != qry.end(); ++it) {
		(*it).getter() >> userid;
		results.push_back(userid);
	}

	user->db.disconnect();

	return results;
}

/*
 * @method      : fileToVector
 * @description : Takes a filename opens it get's the content convert it to a vector and
 * returns the vector.
 * @input       : filename
 * @output      : vector
 *
 */
vector < string > fileToVector(string filename)
{
	vector < string > v;
	string temp;
	fstream fs(filename.c_str(), fstream::in);
	if (fs.is_open() == false) {
		cerr << "\t[-] Error : Unable to open " << filename << endl;
		return v;
	}

	while (fs.eof() == false) {
		getline(fs, temp);
		if (temp.length() != 0)
			v.push_back(temp);
	}

	fs.close();
	return v;
}

/* @method      : status
 * @description : Show how many did we follow, to follow, our followers,
 * and our following.
 * @input       : user
 * @output      : None
 */
bool status(User * user)
{
	string replyMsg, followers, following, reset_time;
	int remaining_hits = 0, hourly_limit = 0;

	if (removeDuplicates(user) == false)
		return false;

	vector < string > tofollow(dbToVector(user, "ToFollow", "userid"));
	vector < string > followed(dbToVector(user, "Followed", "userid"));
	vector < string > unfollowed(dbToVector(user, "UnFollowed", "userid"));

	cout << "\tApplication Status :" << endl;
	cout << "\t\tFollowed   : " << followed.size() << endl;
	cout << "\t\tTo follow  : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed : " << unfollowed.size() << endl;

	if (user->twitterObj.accountVerifyCredGet() == true) {
		cout << "\tAccount Status     :" << endl;
		if (parseLastResponse(user, "user.followers_count",
				      followers) == false) {
			cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << endl;
			return false;
		}
		if (parseLastResponse(user, "user.friends_count", following) ==
		    false) {
			cerr << "\t[-] Error : Unable to find friends_count" <<
			    endl;
			return false;
		}

		cout << "\t\tFollowers  : " << followers << endl;
		cout << "\t\tFollowing  : " << following << endl;
	} else {
		cerr << "\t[-] Error : Unable to get account status." << endl;
		return false;
	}

	// API status
	if (user->twitterObj.accountRateLimitGet() == true) {

		cout << "\tAPI Status : " << endl;
		if (parseLastResponse
		    (user, "hash.remaining-hits", remaining_hits) == false) {
			cerr << "[-] Error : Unable to get hash.remaining-hits"
			    << endl;
			return false;
		}

		if (parseLastResponse
		    (user, "hash.hourly-limit", hourly_limit) == false) {
			cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		if (parseLastResponse(user, "hash.reset-time", reset_time) ==
		    false) {
			cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		cout << "\t\tRemaining hits : " << remaining_hits << endl;
		cout << "\t\tHourly limit   : " << hourly_limit << endl;
		cout << "\t\tReset  at      : " << reset_time << endl;
	} else {
		cerr << "\t[-] Error : Unable to get API status" << endl;
		return false;
	}

	return true;
}

/* @method           : randomize
 * @input            : from, to
 * @output           : from <= random <= to
 */
int randomize(int from, int to)
{
	return rand() % to + from;
}

/*
 * @method           : fileExists
 * @description      : check if file exists
 * @input            : filename
 * @output           : true if file exists, false otherwise
 */
bool fileExists(string filename)
{
	struct stat fi;
	return stat(filename.c_str(), &fi) == 0;
}

/*
 * @method           : parseLastResponse
 * @description      : parse the last web response and get a
 * value of a node, save the value in T v
 * @input            : user, the node to get, T v
 * @output           : true if got the result false otherwise
 */
template < class T > bool parseLastResponse(User * user, string node, T & v)
{
	string replyMsg = "";
	ptree pt;
	user->twitterObj.getLastWebResponse(replyMsg);
	stringstream ss(replyMsg);

	try {
		read_xml(ss, pt);
		v = pt.get < T > (node.c_str());
	}
	catch(exception const &e) {
		return false;
	}
	return true;
}

/*
 * @method      : optionParse
 * @description : Do a specific job depending on the opt
 * @input       : User object, option number
 * @output      : None
 */
void optionParse(User * user, int opt)
{

	string username;
	vector < string > ids;

	switch (opt) {
	case 1:		// Get followers of a user
		{
			cout << "Username : ";
			cin >> username;
			if (removeDuplicates(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;

			}

			ids = getFollowersOf(user, username);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				cerr << "[-] Error : vectorToDB" << endl;
				return;
			}

			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;
		}
		break;
	case 2:		// Get following of a user
		{
			cout << "Username : ";
			cin >> username;
			if (removeDuplicates(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = getFollowingOf(user, username);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				cerr << "[-] Error : vectorToDB" << endl;
				return;
			}

			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;

			break;
		}
	case 3:		// Search
		{
			string query;
			cout << "Enter a phrase : ";
			cin.ignore();
			getline(cin, query);

			if (removeDuplicates(user) == false) {
				cerr << "(Err:Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = search(user, query);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				cerr << "[-] Error : vectorToDB" << endl;
				return;
			}
			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;
		}
		break;
	case 4:		// follow users
		{
			follow(dbToVector(user, "ToFollow", "userid"), user);
		}
		break;
	case 5:		// our status
		{
			status(user);
		}
		break;
	case 6:		// unfollow users
		{
			unfollow(user);
		}
		break;
	case 7:		// Configure
		{
			configure(user);
			cout << "Rerun application to apply changes." << endl;
		}
		break;
	case 8:		// quit
		{
			cout << "\tHave a nice day!" << endl;
			exit(1);
		}
		break;
	default:
		cerr << "\t[-] Error : Invalid option" << endl;
		break;
	}
}

/*
 * @method      : Unfollow
 * @description : Unfollow users who haven't followed me back
 * @input       : user 
 * @output      : None
 */
void unfollow(User * user)
{
	vector < string > followers(getFollowingOf(user, user->username));
	vector < string > result;
	string replyMsg, who;
	bool isfollow = true;
	long unfollowed = 0;
	gotExitSignal = false;

	if (removeDuplicates(user) == false) {
		cerr << "(Err:Unable to remove duplicates)" << endl;
		return;
	}

	/* Install the signal handler */
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		cerr << "(Err:Unable to install signalHandler)" << endl;
		return;
	}
	// Don't do anything if there's no one to unfollow
	if (followers.size() == 0) {
		cerr << "(Err:No one to unfollow)" << endl;
		return;
	}

	/*
	 * Now decide the followers who haven't followed me back
	 * and unfollow them
	 * */
	for (vector < string >::iterator it = followers.begin();
	     it != followers.end() && gotExitSignal != true; it++) {
		user->twitterObj.friendshipShow(*it, true);

		if (parseLastResponse(user,
				      "relationship.source.followed_by",
				      isfollow) == false) {
			cerr <<
			    "(Err: Unable to find relationship.source.followed_by)"
			    << endl;
			break;
		}

		if (isfollow == false) {
			if (parseLastResponse(user,
					      "relationship.target.screen_name",
					      who) == false) {
				cerr <<
				    "(Err:Unable to find relationship.target.screen_name)"
				    << endl;
				break;
			}
			if (user->twitterObj.friendshipDestroy(*it, true)) {
				cout << "Unfollowed " << who;
				unfollowed++;
				result.push_back(*it);
				cleanLine(120);
				sleep(randomize(1, 1));
			} else {
				cout << "Unable to Unfollow " << who;
				cleanLine(120);
			}
		}
		isfollow = true;
	}

	// restart the exit signal flag
	gotExitSignal = false;
	cout << "We have unfollowed " << unfollowed << "/" << followers.size()
	    << endl;
	// write results to db
	if (vectorToDB(user, result, "UnFollowed", "userid") == false)
		cerr << "[-] Error : Unable to write to db" << endl;

	return;
}

/* @method      : twitCurl::~twitCurl
 * @description : Fixes bugs. 
 * @input       : None
 * @output      : None
 */
twitCurl::~twitCurl()
{
	// Do nothing
}

/* @method      : optionShow
 * @description : Show available option to the user
 * @input       : None
 * @output      : None
 */
void optionShow()
{
	cout << "1) Get followers of a user" << endl;
	cout << "2) Get following of a user" << endl;
	cout << "3) Search by " << endl;
	cout << "4) Start following" << endl;
	cout << "5) Status" << endl;
	cout << "6) Unfollow users who haven't followed" << endl;
	cout << "7) Configure" << endl;
	cout << "8) Quit" << endl;
}

/* @method      : For user configuration
 * @description : Configure various options for user
 * @input       : user
 * @output      : false if failed otherwise true
 */
bool configure(User * user)
{
	string address, port, username, password;
	string q;
	int opt = -1;
	cout << "1) Set proxy" << endl;
        cout << "2) Purge To Follow" << endl;
        cout << "3) Purge Followed" << endl;
        cout << "4) Purge Unfollowed" << endl;
        cout << "5) Purge MyFollowers" << endl;
        cout << "6) Pruge all" << endl;
	cout << "7) Return" << endl;
	opt = optionSelect();

	switch (opt) {
	case 1:		// configure proxy
		{
			cin.ignore();
			cout << "address  : ";
			getline(cin, address);
			cout << "port     : ";
			getline(cin, port);
			cout <<
			    "Does this proxy use a username:password [y/n] ? ";
			getline(cin, q);
			if (q == "y" || q == "Y") {
				cout << "username : ";
				getline(cin, username);
				cout << "password : ";
				getline(cin, password);
			}
			if (change_proxy
			    (user, address, port, username, password) == false)
				cerr << "[-] Error Unable to change proxy" <<
				    endl;
		}
		break;
        case 2: // purge to follow
                {
                        if(purgeTableDB(user, "ToFollow") == false)
                                cerr << "[-] Error : Unable toi purge ToFollow" << endl;
                }
                break;
        case 3: // purge followed
                {
                        if(purgeTableDB(user, "Followed") == false)
                                cerr << "[-] Error : Unable to purge Followed" << endl;
                }
                break;
        case 4: // purge unfollowed
                {
                        if(purgeTableDB(user, "UnFollowed") == false)
                                cerr << "[-] Error : Unable to purge UnFollowed" << endl;
                }
                break;
        case 5:
                {
                        if(purgeTableDB(user, "MyFollowers") == false)
                                cerr << "[-] Error : Unable to purge MyFollowers" << endl;
                }
                break;
        case 6: // purge all
                {
                        if(purgeTableDB(user, "ToFollow") == false)
                                cerr << "[-] Error : Unable to purge ToFollow" << endl;
                        if(purgeTableDB(user, "Followed") == false)
                                cerr << "[-] Error : Unable to purge Followed" << endl;
                        if(purgeTableDB(user, "UnFollowed") == false)
                                cerr << "[-] Error : Unable to purge " << endl;
                        if(purgeTableDB(user, "MyFollowers") == false)
                                cerr << "[-] Error : Unable to purge MyFollowers" << endl;
                }
                break;
	case 7:		// return
		break;
	default:
		break;
	}

	return true;
}

/*
 * @method      : purgeTableDB
 * @description : delete everything in a database
 * @input       : user, table
 * @outpt       : false if failed, otherwise true.
 */
bool purgeTableDB(User * user, string table) {
        string q;
        user->db.connect( user->db_name.c_str() );
        q = "DELETE FROM " + table + ";";
        if(user->db.execute( q.c_str() ) != 0)
                return false;
        user->db.disconnect();
        return true;
}

/*
 * @method      : change_proxy
 * @description : change proxy and set it in database
 * @input       : address,port,username,password
 * @output      : false if failed
 */
bool change_proxy(User * user, string address, string port, string username,
		  string password)
{

	string q;

	user->proxy.address = address;
	user->proxy.port = port;
	user->proxy.username = username;
	user->proxy.password = password;

	user->db.connect(user->db_name.c_str());

	// update DB with new proxy
	q = "UPDATE Config SET proxy_address = \"" + user->proxy.address +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET proxy_port = \"" + user->proxy.port +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET proxy_username = \"" + user->proxy.username +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;
	q = "UPDATE Config SET proxy_password = \"" + user->proxy.password +
	    "\" WHERE Id=1;";
	if (user->db.execute(q.c_str()) != 0)
		return false;

	user->db.disconnect();

	return true;
}

/*
 * @method      : getValFromDB
 * @description : Get a value col from a table return vector.
 * @input       : user, table, col
 * @output      : vector of string
 */
vector < string > getValFromDB(User * user, string table, string col)
{
	string val;
	vector < string > vals;
	string q = "SELECT " + col + " FROM " + table + ";";
	user->db.connect(user->db_name.c_str());
	query qry(user->db, q.c_str());

	for (query::iterator it = qry.begin(); it != qry.end(); it++) {
		(*it).getter() >> val;
		vals.push_back(val);
	}

	user->db.disconnect();
	return vals;
}

/*
 * @method      : authenticate
 * @description : authenticate the user
 * @input       : user
 * @output      : true if successful, otherwise false
 */
bool authenticate(User * user)
{
	string q;

	// set twitter user, pass, and consumer {key,secret}
	user->twitterObj.setTwitterUsername(user->username);
	user->twitterObj.setTwitterPassword(user->password);
	user->twitterObj.getOAuth().setConsumerKey(user->consumer_key);
	user->twitterObj.getOAuth().setConsumerSecret(user->consumer_secret);

	// if we already have oauth keys
	if (user->access_token_key.size() && user->access_token_secret.size()) {
		user->twitterObj.getOAuth().setOAuthTokenKey(user->
							     access_token_key);
		user->twitterObj.getOAuth().
		    setOAuthTokenSecret(user->access_token_secret);
		return true;
	} else {		// if we don't
		// get pin
		string authurl, pin;
		user->twitterObj.oAuthRequestToken(authurl);
		cout <<
		    "Visit twitter and authorize the application then enter the PIN."
		    << endl << authurl << endl;
		cout << "PIN : ";
		cin >> pin;
		user->twitterObj.getOAuth().setOAuthPin(pin);
		user->twitterObj.oAuthAccessToken();

		// update database with access keys
		user->twitterObj.getOAuth().getOAuthTokenKey(user->
							     access_token_key);
		user->twitterObj.getOAuth().
		    getOAuthTokenSecret(user->access_token_secret);

		q = "UPDATE Config SET access_key = \"" +
		    user->access_token_key + "\" WHERE Id=1;";
		if (user->db.execute(q.c_str()) != 0)
			return false;
		q = "UPDATE Config SET access_secret = \"" +
		    user->access_token_secret + "\" WHERE Id=1;";
		if (user->db.execute(q.c_str()) != 0)
			return false;
		return true;
	}
	/* OAuth flow ends */

	return false;
}

/* @method      : main
 * @description : main
 * @input       : main
 * @output      : main
 */
int main()
{
	srand(time(NULL));	// random seed

	User *user = new User;
	string replyMsg;
	string temp;

	user->db_name = "cache/db.sql";
	user->consumer_key = "nYFCp8lj4LHqmLTnVHFc0Q";
	user->consumer_secret = "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4";

	/* Initalize database */
	if (initalizeDatabase(user) == false) {
		cerr << "[-] Error : Unable to initalize database" << endl;
		return -1;
	}
	// we should see if the Config table has someone or not
	if (userExistInDB(user) == false) {
		// create the username, password
		if (createUser(user) == false) {
			cerr << "Unable to create user" << endl;
			return -1;
		}
	} else {		// Get all needed values from DB
		user->username = getValFromDB(user, "Config", "username").at(0);
		user->access_token_key =
		    getValFromDB(user, "Config", "access_key").at(0);
		user->access_token_secret =
		    getValFromDB(user, "Config", "access_secret").at(0);
		user->proxy.address =
		    getValFromDB(user, "Config", "proxy_address").at(0);
		user->proxy.port =
		    getValFromDB(user, "Config", "proxy_port").at(0);
		user->proxy.username =
		    getValFromDB(user, "Config", "proxy_username").at(0);
		user->proxy.password =
		    getValFromDB(user, "Config", "proxy_password").at(0);
	}

	/* Set up proxy if found */
	if (!user->proxy.address.empty() && !user->proxy.port.empty()) {
		user->twitterObj.setProxyServerIp(user->proxy.address);
		user->twitterObj.setProxyServerPort(user->proxy.port);
		cout << "[+] Using proxy " << user->proxy.
		    address << ":" << user->proxy.port << endl;
		/* Set password if found */
		if (!user->proxy.username.empty()
		    && !user->proxy.password.empty()) {
			user->twitterObj.setProxyUserName(user->proxy.username);
			user->twitterObj.setTwitterPassword(user->proxy.
							    password);
		}
	}

	/* Authenticate our user */
	if (authenticate(user) == false) {
		cerr << "[-] Failed while authenticating" << endl;
		return -1;
	}

	/* Verifying authentication */
	if (user->twitterObj.accountVerifyCredGet() == true) {
		if (parseLastResponse(user, "user.friends_count",
				      user->following) == false) {
			cerr << "[-] Error : Unable to find user.friends_count"
			    << endl;
			return -1;
		}

		if (parseLastResponse(user, "user.followers_count",
				      user->followers) == false) {
			cerr <<
			    "[-] Error : Unable to find user.followers_count" <<
			    endl;
			return -1;
		}
	} else {
		cerr << "[-] Error : Unable to authenticate." << endl;
		if (!user->proxy.address.empty() && !user->proxy.port.empty()) {
			cout <<
			    "If this is due to misconfiguration you can change it"
			    << endl;
			if (configure(user) == false) {
				cerr << "[-] Error : Unable to configure" <<
				    endl;
				return -1;
			}
			cout << "Rerun the application to apply changes." <<
			    endl;
		}

		return -1;
	}

	cout << "=====================" << endl;
	cout << "Hello @" << user->username << endl;
	cout << "Following : " << user->following << endl;
	cout << "Followers : " << user->followers << endl;
	cout << "=====================" << endl << endl;

	// Before entering the main loop fix the databases
	if (removeDuplicates(user) == false) {
		cerr << "[-] Error : Unable to remove duplicates" << endl;
		return -1;
	}

	/*
	 * Start the loop and do things in here 
	 * signals will be used to avoid some shitty cases
	 **/
	int opt = 0;
	while (opt != 8) {
		optionShow();
		opt = optionSelect();
		optionParse(user, opt);
	}

	return 0;
}

/*
 * @method      : Initalize database
 * @description : It creates necessary tables if they don't exists
 * @input       : User
 * @output      : false if unable to connect to db, true otherwise..
 */
bool initalizeDatabase(User * user)
{

	string query;

	// Connect to database
	if (user->db.connect(user->db_name.c_str()) != 0) {
		return false;
	}
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
	    ("CREATE TABLE Config(Id integer PRIMARY KEY, username text, password text, access_key text, access_secret text, proxy_username text, proxy_password text, proxy_address text, proxy_port text);");
	user->db.disconnect();

	return true;
}

/* @method      : vectorToDB
 * @description : will insert or replace UNIQUE a vector<string>.
 * @input       : User, vector, table, values
 * @output      : true if successful false if unable to connect to db
 * or unable to insert to table.
 */
bool vectorToDB(User * user, vector < string > v, string table, string values)
{
	string query;

	// chose database
	if (user->db.connect(user->db_name.c_str()) == 1)
		return false;

	// Inserting into the database
	if (user->db.execute("BEGIN") == 1)
		return false;
	for (vector < string >::iterator it = v.begin(); it != v.end(); it++) {
		query =
		    "INSERT OR REPLACE INTO " + table + " (" + values +
		    ") VALUES ('" + *it + "');";
		if (user->db.execute(query.c_str()) == 1) {
			cerr << "[-] Error : vectorToDB " << query << endl;
			return false;
		}
	}
	if (user->db.execute("COMMIT") == 1)
		return false;

	user->db.disconnect();

	return true;
}

/*
 * @method      : signalHandler
 * @description : Handler for follow/unfollow to set a exitFlag
 * @input       : n the catched signal number
 * @outpu       : None
 */
void signalHandler(int n)
{
	gotExitSignal = true;
}

/*
 * @method      : cleanLine
 * @description : Back to the first line and erase n characters
 * @input       : n of blanks
 * @output      : None
 */
void cleanLine(int n)
{
	for (int i = 0; i < n; i++)
		cout << " ";
	cout << "\xd";
	flush(cout);
}

/* @method      : follow
 * @description : It will follow a vector of user ids
 * @input       : user ids string vector, and user
 * @output      : None
 */
void follow(vector < string > to_follow, User * user)
{
	string username, error;
	gotExitSignal = false;
	vector < string > followed;
	string replyMsg;
	int ignored = 0;

	if (to_follow.size() == 0) {
		cerr << "(Err:Please add users to follow)" << endl;
		return;
	}
	// remove duplicates
	if (removeDuplicates(user) == false) {
		cerr << "(Err:Unable to remove duplicates)" << endl;
		return;
	}

	cout << to_follow.size() << " to follow" << endl;

	/* Install the signal handler */
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		cerr << "(Err:Unable to install signalHandler)" << endl;
		return;
	}

	for (vector < string >::iterator it = to_follow.begin(); it != to_follow.end() && gotExitSignal != true; it++) {	// { Users to follow

		// follow only those that applies to the
		// by_ratio filter
		if (filter::by_ratio(user, *it) == true) {	// if filter passed
			if (user->twitterObj.friendshipCreate(*it, true) == true) {	// if followed the user
				followed.push_back(*it);
				if (parseLastResponse(user, "user.name", username) == false) {	// if can't get username
					if (parseLastResponse(user, "hash.error", error) == true) {	// get hash error

						// did we reach follow limit ?
						if (string::npos !=
						    error.find
						    ("You are unable to follow more people at this time"))
						{
							cout <<
							    "We have reached the follow limit for today."
							    << endl;
							followed.erase(followed.
								       end());
							break;
						} else	// unhandled error (must handle if need to break)
						{
							cerr << "(Err:" << error
							    << ")";
							cleanLine(120);
							followed.push_back(*it);	// We have followed the user
						}
					}
				} else {	// user followed
					cout << "Followed " << username;
					cleanLine(120);
				}
				// sleep for 1-3 seconds
				//sleep(randomize(1, 2));
			} else {	// unable to create friendship
				cerr << "(Err:Unable to follow)";
				cleanLine(120);
			}
		} else {	// filter ignored someone
			// Did we reach API limit?
			if (parseLastResponse(user, "hash.error", error) ==
			    true) {
				if (string::npos !=
				    error.find("Clients may not make")) {
					cerr <<
					    "The client have reached the API limit, please try again in an hour"
					    << endl;
					break;
				}
			}
			// Who did we ignore ?
			if (parseLastResponse(user, "user.name", username) ==
			    true) {
				cout << "Ignored " << username;
				cleanLine(120);
			}
			ignored++;
		}
	}			// } users to follow

	/* when signal is caught or when block is over */
	gotExitSignal = false;
	if (followed.size() != 0)
		cout << endl << "We have followed " << followed.size() << "/" <<
		    to_follow.size() - ignored << endl;

	if (vectorToDB(user, followed, "Followed", "userid") == false) {
		cerr << "[-] Error : vectorToDB" << endl;
		return;
	}
	if (ignored > 0)
		cout << "\tWe have Ignored : " << ignored << endl;
}

/*
 * @method      : concatVectors
 * @description : This will take src and add it to dest
 * @input       : destination vector, source vector
 * @output      : None
 */
template < class T > void concatVectors(vector < T > &dest, vector < T > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

/*
 * @method      : removeDuplicates
 * @descrption  :
 * This will remove duplicates from the caches
 * 1) it will read the cache content to a vector
 * 2) it will sort and remove uniques from the vector
 * 3) it will delete cache, rewrite the new vector to it.
 * @input       : user 
 * @output      : true if duplicates are removed, otherwise false
 */
bool removeDuplicates(User * user)
{
	vector < string > v_tofollow(dbToVector(user, "ToFollow", "userid"));
	vector < string > v_followed(dbToVector(user, "Followed", "userid"));
	vector < string >
	    v_unfollowed(dbToVector(user, "UnFollowed", "userid"));

	// remove anything in followed from tofollow list
	for (vector < string >::iterator x = v_followed.begin();
	     x != v_followed.end(); x++) {
		v_tofollow.erase(remove
				 (v_tofollow.begin(), v_tofollow.end(), *x),
				 v_tofollow.end());
	}

	// remove anything in unfollowed from tofollow list
	for (vector < string >::iterator x = v_unfollowed.begin();
	     x != v_unfollowed.end(); x++) {
		v_tofollow.erase(remove
				 (v_tofollow.begin(), v_tofollow.end(), *x),
				 v_tofollow.end());
	}

	// Write the new tofollow to ToFollow table
	if (user->db.connect(user->db_name.c_str()) == 1)
		return false;
	if (user->db.execute("DELETE FROM ToFollow;") == 1)
		return false;
	if (vectorToDB(user, v_tofollow, "ToFollow", "userid") == false)
		return false;
	user->db.disconnect();
	return true;
}

/*
 * @method      : getFollowingOf
 * @description : Fetch the following of a user
 * and create a vector of their userids and return that vector
 * @input       : user, username
 * @output      : vector of userIDs of the username's following.
 *
 * Doesn't support next_cursor
 */
vector < string > getFollowingOf(User * user, string username)
{
	string replyMsg;
        string next_cursor = "-1";
	string err;
	vector < string > ids;
	cout << "Getting following of @" << username << endl;

        do {
	if (user->twitterObj.friendsIdsGet(next_cursor, username, false) == true) {
		user->twitterObj.getLastWebResponse(replyMsg);
		ptree pt;
		stringstream ss(replyMsg);
		read_xml(ss, pt);

		/* Catched and error ? */
		if (parseLastResponse(user, "hash.error", err) == true) {
			cerr << "\t" << err << endl;
			return ids;
		}

                /* Get next cursor */
                parseLastResponse(user, "id_list.next_cursor", next_cursor);

		try {
			BOOST_FOREACH(ptree::value_type & v,
				      pt.get_child("id_list.ids"))
			    ids.push_back(v.second.data());
		}
		catch(exception const &e) {
			cerr << "\t[-] Error : GetFollowingOf Exception" <<
			    endl;
			return ids;
		}
	} else {
		cerr << "\t[-] Error : Failed to get following of @" << username
		    << endl;
		return ids;
	}
        } while(next_cursor != "0");

	return ids;
}

/*
 * @method      : getFollowersOf
 * @description : Get the followers of a user
 * and create a vector of their userIDs and return that vector
 * @input       : user, username
 * @output      : vector of userIDs of the username's followers
 */
vector < string > getFollowersOf(User * user, string username)
{
	string replyMsg;
	string next_cursor = "-1";
        string err;
	vector < string > ids;
	ptree pt;

	cout << "\tGetting followers of @" << username << endl;

        do {
	if (user->twitterObj.followersIdsGet(next_cursor, username, false) == true) {
		user->twitterObj.getLastWebResponse(replyMsg);
		stringstream ss(replyMsg);
		read_xml(ss, pt);

		/* Catched error ? */
		if (parseLastResponse(user, "hash.error", err) == true) {
			cerr << "\t" << err << endl;
			return ids;
		}

                /* Get next cursor */
                parseLastResponse(user, "id_list.next_cursor", next_cursor);

		try {
			BOOST_FOREACH(ptree::value_type & v,
				      pt.get_child("id_list.ids"))
			    ids.push_back(v.second.data());
		}
		catch(exception const &e) {
			cerr << "\t[-] Error : getFollowersOf Exception"
			    << endl;
			return ids;
		}
	} else {
		cerr << "\t[-] Error : Failed to get friendsIdsGet from @" <<
		    username << endl;
	}
        } while(next_cursor != "0");

	return ids;
}
