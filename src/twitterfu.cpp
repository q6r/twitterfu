#include "twitterfu.h"

/* Globals
 */
bool gotExitSignal = false;
//using boost::property_tree:: boost::property_tree::ptree;

/* @method      : search
 * @description : Will take a query and return a std::vector of
 * user ids to be added to the to follow list. This function
 * is limited to 15 results since pages are not supported
 * by twitCurl.
 * @input       : User, query
 * @output      : std::vector of user ids
 */
std::vector < std::string > search(User * user, std::string query)
{
	std::string jsonResult;
	boost::property_tree::ptree pt;
	std::vector < std::string > ids;

	// replace all spaces with %20
	for (size_t pos = query.find(' '); pos != std::string::npos;
	     pos = query.find(' ', pos))
		query.replace(pos, 1, "%20");

	// send the search query
	if (user->twitterObj.search(query) == false)
		return ids;

	// Get results, parse then and push to ids std::vector
	user->twitterObj.getLastWebResponse(jsonResult);
	std::stringstream ss(jsonResult);

	try {
		read_json(ss, pt);
		BOOST_FOREACH(const  boost::property_tree::ptree::value_type & child,
			      pt.get_child("results")) {
			ids.push_back(child.second.get < std::string >
				      ("from_user_id"));
		}
	} catch(std::exception const &e) {
		std::cerr << "(Err:Unable to parse json)" << std::endl;
		return ids;
	}

	return ids;
}

/*
 * @method      : optionSelect
 * @output      : opt
 */
int optionSelect()
{
	int opt = -1;

	std::cout << "> ";
	std::cin >> opt;

	if (std::cin.fail() == true) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits < std::streamsize >::max(), 0x0a);
		return -1;
	}

	return opt;
}

/* @method      : createUser
 * @description : Will std::cin some information
 * and add them to the database
 * @input       : user
 * @output      : true if successfuly inserted user to database
 * otherwise false;
 */
bool createUser(User * user)
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

/* @method      : userExistInDB
 * @description : Check if there's a user in the DB.
 * @input       : User
 * @output      : true if exists otherwise false
 */
bool userExistInDB(User * user)
{

	user->db.connect(user->db_name.c_str());
	sqlite3pp::query qry(user->db, "SELECT * from Config;");

	if (qry.begin() == qry.end())
		return false;

	user->db.disconnect();
	return true;
}

/* @method      : dbToVector
 * @description : Reads a specific table and return the results in the value
 * as a std::vector of std::strings
 * @input       : user, table, value
 * @output      : std::vector<std::string> of select value from table;
 */
std::vector < std::string > dbToVector(User * user, std::string table, std::string value)
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

/*
 * @method      : fileToVector
 * @description : Takes a filename opens it get's the content convert it to a std::vector and
 * returns the std::vector.
 * @input       : filename
 * @output      : std::vector
 *
 */
std::vector < std::string > fileToVector(std::string filename)
{
	std::vector < std::string > v;
	std::string temp;
	std::fstream fs(filename.c_str(), std::fstream::in);

	if (fs.is_open() == false) {
		std::cerr << "\t[-] Error : Unable to open " << filename << std::endl;
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
 * @description : Show database, account and API status.
 * @input       : user
 * @output      : true if successful otherwise false
 */
bool status(User * user)
{
	std::string result, followers, following, reset_time;
	int remaining_hits = 0, hourly_limit = 0;

	std::vector < std::string > tofollow(dbToVector(user, "ToFollow", "userid"));
	std::vector < std::string > followed(dbToVector(user, "Followed", "userid"));
	std::vector < std::string > unfollowed(dbToVector(user, "UnFollowed", "userid"));
        std::vector < std::string > myfollowers(dbToVector(user, "MyFollowers", "userid"));

	std::cout << "\tDatabase Status :" << std::endl;
	std::cout << "\t\tFollowed     : " << followed.size() << std::endl;
	std::cout << "\t\tTo follow    : " << tofollow.size() << std::endl;
	std::cout << "\t\tUnfollowed   : " << unfollowed.size() << std::endl;
        std::cout << "\t\tMy followers : " << myfollowers.size() << std::endl;

	if (user->twitterObj.accountVerifyCredGet() == true) {
		std::cout << "\tAccount Status     :" << std::endl;
		if (parseLastResponse(user, "user.followers_count",
				      followers) == false) {
			std::cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << std::endl;
			return false;
		}
		if (parseLastResponse(user, "user.friends_count", following) ==
		    false) {
			std::cerr << "\t[-] Error : Unable to find friends_count" <<
			    std::endl;
			return false;
		}

		std::cout << "\t\tFollowers  : " << followers << std::endl;
		std::cout << "\t\tFollowing  : " << following << std::endl;
	} else {
		std::cerr << "\t[-] Error : Unable to get account status." << std::endl;
		return false;
	}

	// API status
	if (user->twitterObj.accountRateLimitGet() == true) {

		std::cout << "\tAPI Status : " << std::endl;
		if (parseLastResponse
		    (user, "hash.remaining-hits", remaining_hits) == false) {
			std::cerr << "[-] Error : Unable to get hash.remaining-hits"
			    << std::endl;
			return false;
		}

		if (parseLastResponse
		    (user, "hash.hourly-limit", hourly_limit) == false) {
			std::cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    std::endl;
			return false;
		}

		if (parseLastResponse(user, "hash.reset-time", reset_time) ==
		    false) {
			std::cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    std::endl;
			return false;
		}

		std::cout << "\t\tRemaining hits : " << remaining_hits << std::endl;
		std::cout << "\t\tHourly limit   : " << hourly_limit << std::endl;
		std::cout << "\t\tReset  at      : " << reset_time << std::endl;
	} else {
		std::cerr << "\t[-] Error : Unable to get API status" << std::endl;
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
bool fileExists(std::string filename)
{
	struct stat fi;
	return stat(filename.c_str(), &fi) == 0;
}

/*
 * @method           : parseLastResponse
 * @description      : parse the last web response and get a
 * value of a node, save the value in T v
 * @input            : user, node, v
 * @output           : true if got the result false otherwise
 */
template < class T > bool parseLastResponse(User * user, std::string node, T & v)
{
	std::string result = "";
	 boost::property_tree::ptree pt;
	user->twitterObj.getLastWebResponse(result);
	std::stringstream ss(result);

	try {
		read_xml(ss, pt);
		v = pt.get < T > (node.c_str());
	}
	catch(std::exception const &e) {
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
	std::string username;
	std::vector < std::string > ids;

	switch (opt) {
	case 1:		// Get followers of a user
		{
			std::cout << "Username : ";
			std::cin >> username;

			if (removeDuplicates(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;

			}

			ids = getFollowersOf(user, username);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : vectorToDB" << std::endl;
				return;
			}

			std::cout << "We have added " << ids.size() <<
			    " new to follow" << std::endl;
		}
		break;
	case 2:		// Get following of a user
		{
			std::cout << "Username : ";
			std::cin >> username;

			if (removeDuplicates(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

			ids = getFollowingOf(user, username);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : vectorToDB" << std::endl;
				return;
			}

			std::cout << "We have added " << ids.size() <<
			    " new to follow" << std::endl;

			break;
		}
	case 3:		// Search
		{
			std::string query;
			std::cout << "Enter a phrase : ";
			std::cin.ignore();
			getline(std::cin, query);

			if (removeDuplicates(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

			ids = search(user, query);

			if (vectorToDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : vectorToDB" << std::endl;
				return;
			}
			std::cout << "We have added " << ids.size() <<
			    " new to follow" << std::endl;
		}
		break;
	case 4:		// follow users
		{
			if (removeDuplicates(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

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
			std::cout << "Rerun application to apply changes." << std::endl;
		}
		break;
	case 8:		// quit
		{
			std::cout << "\tHave a nice day!" << std::endl;
			exit(1);
		}
		break;
	default:
		std::cerr << "\t[-] Error : Invalid option" << std::endl;
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
	std::vector < std::string > followers(getFollowingOf(user, user->username));
	std::vector < std::string >::iterator it;
	std::vector < std::string > result;
	std::string who;
	bool isfollow = true;
	long unfollowed = 0;
	gotExitSignal = false;

	// Don't do anything if there's no one to unfollow
	if (followers.size() == 0) {
		std::cerr << "(Err:No one to unfollow)" << std::endl;
		return;
	}

	// Install the signal handler
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		std::cerr << "(Err:Unable to install signalHandler)" << std::endl;
		return;
	}

	/*
	 * Now decide the followers who haven't followed me back
	 * and unfollow them
	 * */
	for (it = followers.begin();
	     it != followers.end() && gotExitSignal != true; it++) {

		user->twitterObj.friendshipShow(*it, true);

		if (parseLastResponse(user,
				      "relationship.source.followed_by",
				      isfollow) == false) {
			std::cerr <<
			    "(Err: Unable to find relationship.source.followed_by)"
			    << std::endl;
			break;
		}

		if (isfollow == false) {
			if (parseLastResponse(user,
					      "relationship.target.screen_name",
					      who) == false) {
				std::cerr <<
				    "(Err:Unable to find relationship.target.screen_name)"
				    << std::endl;
				break;
			}
			if (user->twitterObj.friendshipDestroy(*it, true)) {
				std::cout << "Unfollowed " << who;
				unfollowed++;
				result.push_back(*it);
				cleanLine(120);
				sleep(randomize(1, 1));
			} else {
				std::cout << "Unable to Unfollow " << who;
				cleanLine(120);
			}
		}
		isfollow = true;
	}

	// restart the exit signal flag
	gotExitSignal = false;
	std::cout << "We have unfollowed " << unfollowed << "/" << followers.size()
	    << std::endl;

	// write results to db
	if (vectorToDB(user, result, "UnFollowed", "userid") == false)
		std::cerr << "[-] Error : Unable to write to db" << std::endl;
}

/* @method      : twitCurl::~twitCurl
 */
twitCurl::~twitCurl()
{
}

/* @method      : optionShow
 * @description : Show available option to the user
 */
void optionShow()
{
	std::cout << "1) Get followers of a user" << std::endl;
	std::cout << "2) Get following of a user" << std::endl;
	std::cout << "3) Search by " << std::endl;
	std::cout << "4) Start following" << std::endl;
	std::cout << "5) Status" << std::endl;
	std::cout << "6) Unfollow users who haven't followed" << std::endl;
	std::cout << "7) Configure" << std::endl;
	std::cout << "8) Quit" << std::endl;
}

/* @method      : For user configuration
 * @description : Configure various options for user
 * @input       : user
 * @output      : false if failed otherwise true
 */
bool configure(User * user)
{
	std::string address, port, username, password, q;
	int opt = -1;

	std::cout << "1) Set proxy" << std::endl;
	std::cout << "2) Filters" << std::endl;
	std::cout << "3) Purge To Follow" << std::endl;
	std::cout << "4) Purge Followed" << std::endl;
	std::cout << "5) Purge Unfollowed" << std::endl;
	std::cout << "6) Purge MyFollowers" << std::endl;
	std::cout << "7) Pruge all" << std::endl;
	std::cout << "8) Return" << std::endl;

	opt = optionSelect();

	switch (opt) {
	case 1:		// configure proxy
		{
			std::cin.ignore();

			std::cout << "address  : ";
			getline(std::cin, address);

			std::cout << "port     : ";
			getline(std::cin, port);

			std::cout <<
			    "Does this proxy use a username:password [y/n] ? ";
			getline(std::cin, q);

			if (q == "y" || q == "Y") {
				std::cout << "username : ";
				getline(std::cin, username);
				std::cout << "password : ";
				getline(std::cin, password);
			}

			if (change_proxy
			    (user, address, port, username, password) == false)
				std::cerr << "[-] Error Unable to change proxy" <<
				    std::endl;
		}
		break;
	case 2:		// filters
		{
			filter::filter_list(user);
		}
		break;
	case 3:		// purge to follow
		{
			if (purgeTableDB(user, "ToFollow") == false)
				std::cerr << "[-] Error : Unable toi purge ToFollow"
				    << std::endl;
		}
		break;
	case 4:		// purge followed
		{
			if (purgeTableDB(user, "Followed") == false)
				std::cerr << "[-] Error : Unable to purge Followed"
				    << std::endl;
		}
		break;
	case 5:		// purge unfollowed
		{
			if (purgeTableDB(user, "UnFollowed") == false)
				std::cerr << "[-] Error : Unable to purge UnFollowed"
				    << std::endl;
		}
		break;
	case 6:
		{
			if (purgeTableDB(user, "MyFollowers") == false)
				std::cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    std::endl;
		}
		break;
	case 7:		// purge all
		{
			if (purgeTableDB(user, "ToFollow") == false)
				std::cerr << "[-] Error : Unable to purge ToFollow"
				    << std::endl;

			if (purgeTableDB(user, "Followed") == false)
				std::cerr << "[-] Error : Unable to purge Followed"
				    << std::endl;

			if (purgeTableDB(user, "UnFollowed") == false)
				std::cerr << "[-] Error : Unable to purge " << std::endl;

			if (purgeTableDB(user, "MyFollowers") == false)
				std::cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    std::endl;
		}
		break;
	case 8:		// return
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
bool purgeTableDB(User * user, std::string table)
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
 * @method      : change_proxy
 * @description : change proxy and set it in database
 * @input       : address,port,username,password
 * @output      : false if failed
 */
bool change_proxy(User * user, std::string address, std::string port, std::string username,
		  std::string password)
{

	std::string q;

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
 * @description : Get a value col from a table return std::vector.
 * @input       : user, table, col
 * @output      : std::vector of std::string
 */
std::vector < std::string > getValFromDB(User * user, std::string table, std::string col)
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

/*
 * @method      : authenticate
 * @description : authenticate the user
 * @input       : user
 * @output      : true if successful, otherwise false
 */
bool authenticate(User * user)
{
	std::string q, authurl, pin;

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
		user->twitterObj.oAuthRequestToken(authurl);
		std::cout <<
		    "Visit twitter and authorize the application then enter the PIN."
		    << std::endl << authurl << std::endl;
		std::cout << "PIN : ";
		std::cin >> pin;
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

	return false;
}

/* @method      : main
 */
int main()
{
	std::vector < std::string > myFollowers;
	User *user = new User;
	std::string result, temp, query;
	int opt;

	srand(time(NULL));	// random seed

	user->db_name = "cache/db.sql";
	user->consumer_key = "nYFCp8lj4LHqmLTnVHFc0Q";
	user->consumer_secret = "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4";

	/* Initalize database */
	if (initalizeDatabase(user) == false) {
		std::cerr << "[-] Error : Unable to initalize database" << std::endl;
		return -1;
	}
	// we should see if the Config table has someone or not
	if (userExistInDB(user) == false) {
		// create the username, password
		if (createUser(user) == false) {
			std::cerr << "Unable to create user" << std::endl;
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
		user->timezone = getValFromDB(user, "Config", "timezone").at(0);
	}

	/* Set up proxy if found */
	if (!user->proxy.address.empty() && !user->proxy.port.empty()) {
		user->twitterObj.setProxyServerIp(user->proxy.address);
		user->twitterObj.setProxyServerPort(user->proxy.port);
		std::cout << "[+] Using proxy " << user->proxy.
		    address << ":" << user->proxy.port << std::endl;
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
		std::cerr << "[-] Failed while authenticating" << std::endl;
		return -1;
	}

	/* Verifying authentication */
	if (user->twitterObj.accountVerifyCredGet() == true) {
		// get following
		if (parseLastResponse(user, "user.friends_count",
				      user->following) == false) {
			std::cerr << "[-] Error : Unable to find user.friends_count"
			    << std::endl;
			return -1;
		}
		// get followers
		if (parseLastResponse(user, "user.followers_count",
				      user->followers) == false) {
			std::cerr <<
			    "[-] Error : Unable to find user.followers_count" <<
			    std::endl;
			return -1;
		}
		// set timezone if not set
		if (user->timezone.empty()) {
			if (parseLastResponse
			    (user, "user.time_zone", user->timezone) == false) {
				std::cerr << "[-] Error : Unable to find timezone" <<
				    std::endl;
				return -1;
			} else {
				user->db.connect(user->db_name.c_str());
				query =
				    "UPDATE Config SET timezone = \"" +
				    user->timezone + "\";";
				if (user->db.execute(query.c_str()) != 0) {
					std::cerr << "Unable to update timezone" <<
					    std::endl;
				}
				user->db.disconnect();
				std::cout << "We have set the timezone to " <<
				    user->timezone << std::endl;
			}
		}
	} else {
		std::cerr << "[-] Error : Unable to authenticate." << std::endl;
		if (!user->proxy.address.empty() && !user->proxy.port.empty()) {
			std::cout <<
			    "If this is due to misconfiguration you can change it"
			    << std::endl;
			if (configure(user) == false) {
				std::cerr << "[-] Error : Unable to configure" <<
				    std::endl;
				return -1;
			}
			std::cout << "Rerun the application to apply changes." <<
			    std::endl;
		}

		return -1;
	}

	std::cout << "=====================" << std::endl;
	std::cout << "Hello @" << user->username << std::endl;
	std::cout << "Following : " << user->following << std::endl;
	std::cout << "Followers : " << user->followers << std::endl;
	std::cout << "=====================" << std::endl << std::endl;

	/* We shall get our followers */
	myFollowers = getFollowersOf(user, user->username);
	if (myFollowers.size() != 0) {
		std::cout << "Adding a result of " << myFollowers.size() <<
		    " to MyFollowers;" << std::endl;
		if (vectorToDB(user, myFollowers, "MyFollowers", "userid") ==
		    false) {
			std::cerr << "[-] Error : Unable to vectorToDB" << std::endl;
		}
	}
	// Before entering the main loop fix the databases
	if (removeDuplicates(user) == false) {
		std::cerr << "[-] Error : Unable to remove duplicates" << std::endl;
		return -1;
	}

	opt = 0;
	while (opt != 8) {
		optionShow();
		opt = optionSelect();
		optionParse(user, opt);
	}

	return 0;
}

/*
 * @method      : initalizeDatabase
 * @description : It creates necessary tables if they don't exists
 * @input       : User
 * @output      : false if unable to connect to db, true otherwise.
 */
bool initalizeDatabase(User * user)
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

/* @method      : vectorToDB
 * @description : will insert or replace UNIQUE a std::vector<std::string>.
 * @input       : User, std::vector, table, values
 * @output      : true if successful false if unable to connect to db
 * or unable to insert to table.
 */
bool vectorToDB(User * user, std::vector < std::string > v, std::string table, std::string values)
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
			std::cerr << "[-] Error : vectorToDB " << query << std::endl;
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
		std::cout << " ";
	std::cout << "\xd";
	flush(std::cout);
}

/* @method      : follow
 * @description : It will follow a std::vector of user ids
 * @input       : user ids std::string std::vector, and user
 * @output      : None
 */
void follow(std::vector < std::string > to_follow, User * user)
{
	gotExitSignal = false;
	std::string username, error, result;
	std::vector < std::string > followed;
	std::vector < std::string >::iterator it;
	int ignored = 0;

	if (to_follow.size() == 0) {
		std::cerr << "(Err:Please add users to follow)" << std::endl;
		return;
	}
	// remove duplicates
	if (removeDuplicates(user) == false) {
		std::cerr << "(Err:Unable to remove duplicates)" << std::endl;
		return;
	}

	std::cout << to_follow.size() << " to follow" << std::endl;

	/* Install the signal handler */
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		std::cerr << "(Err:Unable to install signalHandler)" << std::endl;
		return;
	}

	for (it = to_follow.begin(); it != to_follow.end() && gotExitSignal != true; it++) {	// { Users to follow

		// follow only those that applies to the
		// by_ratio filter
		if (filter::main(user, *it) == true) {	// if filter passed
			if (user->twitterObj.friendshipCreate(*it, true) == true) {	// if followed the user
				followed.push_back(*it);
				if (parseLastResponse(user, "user.name", username) == false) {	// if can't get username
					if (parseLastResponse(user, "hash.error", error) == true) {	// get hash error

						// did we reach follow limit ?
						if (std::string::npos !=
						    error.find
						    ("You are unable to follow more people at this time"))
						{
							std::cout <<
							    "We have reached the follow limit for today."
							    << std::endl;
							followed.erase(followed.
								       end());
							break;
						} else	// unhandled error (must handle if need to break)
						{
							std::cerr << "(Err:" << error
							    << ")";
							cleanLine(120);
							followed.push_back(*it);	// We have followed the user
						}
					}
				} else {	// user followed
					std::cout << "Followed " << username;
					cleanLine(120);
				}
				// sleep for 1-3 seconds
				//sleep(randomize(1, 2));
			} else {	// unable to create friendship
				std::cerr << "(Err:Unable to follow)";
				cleanLine(120);
			}
		} else {	// filter ignored someone
			// Did we reach API limit?
			if (parseLastResponse(user, "hash.error", error) ==
			    true) {
				if (std::string::npos !=
				    error.find("Clients may not make")) {
					std::cerr <<
					    "The client have reached the API limit, please try again in an hour"
					    << std::endl;
					break;
				}
			}
			// Who did we ignore ?
			if (parseLastResponse(user, "user.name", username) ==
			    true) {
				std::cout << "Ignored " << username;
				cleanLine(120);
			}
			ignored++;
		}
	}			// } users to follow

	/* when signal is caught or when block is over */
	gotExitSignal = false;
	if (followed.size() != 0)
		std::cout << std::endl << "We have followed " << followed.size() << "/" <<
		    to_follow.size() - ignored << std::endl;

	if (vectorToDB(user, followed, "Followed", "userid") == false) {
		std::cerr << "[-] Error : vectorToDB" << std::endl;
		return;
	}
	if (ignored > 0)
		std::cout << "\tWe have Ignored : " << ignored << std::endl;
}

/*
 * @method      : concatVectors
 * @description : This will take src and add it to dest
 * @input       : destination std::vector, source std::vector
 * @output      : None
 */
template < class T > void concatVectors(std::vector < T > &dest, std::vector < T > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

/*
 * @method      : removeDuplicates
 * @descrption  :
 * This will remove duplicates from the caches
 * 1) it will read the cache content to a std::vector
 * 2) it will sort and remove uniques from the std::vector
 * 3) it will delete cache, rewrite the new std::vector to it.
 * @input       : user 
 * @output      : true if duplicates are removed, otherwise false
 */
bool removeDuplicates(User * user)
{
	std::vector < std::string > v_tofollow(dbToVector(user, "ToFollow", "userid"));
	std::vector < std::string > v_followed(dbToVector(user, "Followed", "userid"));
	std::vector < std::string >
	    v_unfollowed(dbToVector(user, "UnFollowed", "userid"));
	std::vector < std::string >
	    v_myfollowers(dbToVector(user, "MyFollowers", "userid"));
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

	if (vectorToDB(user, v_tofollow, "ToFollow", "userid") == false)
		return false;

	user->db.disconnect();

	return true;
}

/*
 * @method      : getFollowingOf
 * @description : Fetch the following of a user
 * and create a std::vector of their userids and return that std::vector
 * @input       : user, username
 * @output      : std::vector of userIDs of the username's following.
 */
std::vector < std::string > getFollowingOf(User * user, std::string username)
{
	std::string result, err, next_cursor;
	std::vector < std::string > ids;
	next_cursor = "-1";

	std::cout << "[+] Getting following of @" << username << std::endl;

	do {
		if (user->twitterObj.friendsIdsGet(next_cursor, username,
						   false) == true) {
			user->twitterObj.getLastWebResponse(result);
			 boost::property_tree::ptree pt;
			std::stringstream ss(result);
			read_xml(ss, pt);

			/* Catched and error ? */
			if (parseLastResponse(user, "hash.error", err) == true) {
				std::cerr << "[-] " << err << std::endl;
				return ids;
			}

			/* Get next cursor */
			parseLastResponse(user, "id_list.next_cursor",
					  next_cursor);

			try {
				BOOST_FOREACH( boost::property_tree::ptree::value_type & v,
					      pt.get_child("id_list.ids"))
				    ids.push_back(v.second.data());
			}
			catch(std::exception const &e) {
				std::cerr << "[-] GetFollowingOf Exception" << std::endl;
				return ids;
			}
		} else {
			std::cerr << "[-] Failed to get following of @" <<
			    username << std::endl;
			return ids;
		}
	} while (next_cursor != "0");

	return ids;
}

/*
 * @method      : getFollowersOf
 * @description : Get the followers of a user
 * and create a std::vector of their userIDs and return that std::vector
 * @input       : user, username
 * @output      : std::vector of userIDs of the username's followers
 */
std::vector < std::string > getFollowersOf(User * user, std::string username)
{
	std::string result, err, next_cursor;
	std::vector < std::string > ids;
	 boost::property_tree::ptree pt;

	next_cursor = "-1";

	std::cout << "[+] Getting followers of @" << username << std::endl;

	do {
		if (user->twitterObj.followersIdsGet(next_cursor, username,
						     false) == true) {
			user->twitterObj.getLastWebResponse(result);
			std::stringstream ss(result);
			read_xml(ss, pt);

			/* Catched error ? */
			if (parseLastResponse(user, "hash.error", err) == true) {
				std::cerr << "[-] " << err << std::endl;
				return ids;
			}

			/* Get next cursor */
			parseLastResponse(user, "id_list.next_cursor",
					  next_cursor);

			try {
				BOOST_FOREACH( boost::property_tree::ptree::value_type & v,
					      pt.get_child("id_list.ids"))
				    ids.push_back(v.second.data());
			}
			catch(std::exception const &e) {
				std::cerr << "[-] getFollowersOf Exception" << std::endl;
				return ids;
			}
		} else {
			std::cerr <<
			    "[-] Failed to get friendsIdsGet from @"
			    << username << std::endl;
		}
	} while (next_cursor != "0");

	return ids;
}
