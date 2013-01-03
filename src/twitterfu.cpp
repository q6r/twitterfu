/* Twitterfu
 *
 * This file is part of Twitterfu.
 *
 * Twitterfu is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Twitterfu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Twitterfu. If not, see <http://www.gnu.org/licenses/>.
 */

#include "twitterfu.h"

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
		std::cin.ignore(std::numeric_limits < std::streamsize >::max(),
				0x0a);
		return -1;
	}

	return opt;
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
		std::cerr << "\t[-] Error : Unable to open " << filename <<
		    std::endl;
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

			if (database::removeDuplicatesInToFollow(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;

			}

			ids = action::getFollowers(user, username);

			if (database::toDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : database::toDB" <<
				    std::endl;
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

			if (database::removeDuplicatesInToFollow(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

			ids = action::getFollowing(user, username);

			if (database::toDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : database::toDB" <<
				    std::endl;
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

			if (database::removeDuplicatesInToFollow(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

			ids = action::search(user, query);

			if (database::toDB(user, ids, "ToFollow", "userid") ==
			    false) {
				std::cerr << "[-] Error : database::toDB" <<
				    std::endl;
				return;
			}
			std::cout << "We have added " << ids.size() <<
			    " new to follow" << std::endl;
		}
		break;
	case 4:		// follow users
		{
			if (database::removeDuplicatesInToFollow(user) == false) {
				std::cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    std::endl;
				return;
			}

			action::follow(database::toVector
				       (user, "ToFollow", "userid"), user);
		}
		break;
	case 5:		// our status
		{
			action::status(user);
		}
		break;
	case 6:		// unfollow users
		{
			action::unfollow(user);
		}
		break;
	case 7:		// Configure
		{
			configure(user);
			std::cout << "Rerun application to apply changes." <<
			    std::endl;
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
				std::cerr << "[-] Error Unable to change proxy"
				    << std::endl;
		}
		break;
	case 2:		// filters
		{
			filter::filter_list(user);
		}
		break;
	case 3:		// purge to follow
		{
			if (database::purgeTable(user, "ToFollow") == false)
				std::cerr <<
				    "[-] Error : Unable toi purge ToFollow" <<
				    std::endl;
		}
		break;
	case 4:		// purge followed
		{
			if (database::purgeTable(user, "Followed") == false)
				std::cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    std::endl;
		}
		break;
	case 5:		// purge unfollowed
		{
			if (database::purgeTable(user, "UnFollowed") == false)
				std::cerr <<
				    "[-] Error : Unable to purge UnFollowed" <<
				    std::endl;
		}
		break;
	case 6:
		{
			if (database::purgeTable(user, "MyFollowers") == false)
				std::cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    std::endl;
		}
		break;
	case 7:		// purge all
		{
			if (database::purgeTable(user, "ToFollow") == false)
				std::cerr <<
				    "[-] Error : Unable to purge ToFollow" <<
				    std::endl;

			if (database::purgeTable(user, "Followed") == false)
				std::cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    std::endl;

			if (database::purgeTable(user, "UnFollowed") == false)
				std::cerr << "[-] Error : Unable to purge " <<
				    std::endl;

			if (database::purgeTable(user, "MyFollowers") == false)
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
 * @method      : change_proxy
 * @description : change proxy and set it in database
 * @input       : address,port,username,password
 * @output      : false if failed
 */
bool change_proxy(User * user, std::string address, std::string port,
		  std::string username, std::string password)
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
	std::string error;
	std::vector < std::string > myFollowers;
	User *user = new User;
	std::string result, temp, query;
	int opt, remainingHits;
        struct passwd *pw = getpwuid( getuid() );

	srand(time(NULL));	// random seed

        /* get users directory */
        user->db_name = pw->pw_dir;
        user->db_name += "/.twitterfu.sql";
	user->consumer_key = "nYFCp8lj4LHqmLTnVHFc0Q";
	user->consumer_secret = "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4";

	/* Initalize database */
	if (database::initalize(user) == false) {
		std::cerr << "[-] Error : Unable to initalize database" <<
		    std::endl;
		return -1;
	}
	// we should see if the Config table has someone or not
	if (database::userExist(user) == false) {
		// create the username, password
		if (database::createUser(user) == false) {
			std::cerr << "Unable to create user" << std::endl;
			return -1;
		}
	} else {		// Get all needed values from DB
		user->username =
		    database::getVal(user, "Config", "username").at(0);
		user->access_token_key =
		    database::getVal(user, "Config", "access_key").at(0);
		user->access_token_secret =
		    database::getVal(user, "Config", "access_secret").at(0);
		user->proxy.address =
		    database::getVal(user, "Config", "proxy_address").at(0);
		user->proxy.port =
		    database::getVal(user, "Config", "proxy_port").at(0);
		user->proxy.username =
		    database::getVal(user, "Config", "proxy_username").at(0);
		user->proxy.password =
		    database::getVal(user, "Config", "proxy_password").at(0);
		user->timezone =
		    database::getVal(user, "Config", "timezone").at(0);
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

	/* If we don't have enough hits suggest using a proxy
	 * and exit 
	 **/
	remainingHits = action::getRemainingHits(user);
	if (remainingHits == 0) {
		std::cerr <<
		    "[-] Error : You have reached the limit, maybe using a proxy might help"
		    << std::endl;
		if (configure(user) == false) {
			std::cerr << "[-] Error : Unable to configure" <<
			    std::endl;
			return -1;
		}
		return -1;
	}

	/* Verifying authentication */
	if (user->twitterObj.accountVerifyCredGet() == true) {
		// get following
		if (action::lastResponse(user, "user.friends_count",
					 user->following) == false) {
			std::cerr <<
			    "[-] Error : Unable to find user.friends_count" <<
			    std::endl;
			return -1;
		}
		// get followers
		if (action::lastResponse(user, "user.followers_count",
					 user->followers) == false) {
			std::cerr <<
			    "[-] Error : Unable to find user.followers_count" <<
			    std::endl;
			return -1;
		}
		// set timezone if not set
		if (user->timezone.empty()) {
			if (action::lastResponse
			    (user, "user.time_zone", user->timezone) == false) {
				std::cerr <<
				    "[-] Error : Unable to find timezone" <<
				    std::endl;
				return -1;
			} else {
				user->db.connect(user->db_name.c_str());
				query =
				    "UPDATE Config SET timezone = \"" +
				    user->timezone + "\";";
				if (user->db.execute(query.c_str()) != 0) {
					std::cerr << "Unable to update timezone"
					    << std::endl;
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
				std::cerr << "[-] Error : Unable to configure"
				    << std::endl;
				return -1;
			}
			std::cout << "Rerun the application to apply changes."
			    << std::endl;
		}

		return -1;
	}

	std::cout << "=====================" << std::endl;
	std::cout << "Hello @" << user->username << std::endl;
	std::cout << "Following : " << user->following << std::endl;
	std::cout << "Followers : " << user->followers << std::endl;
	std::cout << "=====================" << std::endl << std::endl;

	/* We shall get our followers */
	myFollowers = action::getFollowers(user, user->username);
	if (myFollowers.size() != 0) {
		std::cout << "Adding a result of " << myFollowers.size() <<
		    " to MyFollowers;" << std::endl;
		if (database::toDB(user, myFollowers, "MyFollowers", "userid")
		    == false) {
			std::cerr << "[-] Error : Unable to database::toDB" <<
			    std::endl;
		}
	}
	// Before entering the main loop fix the databases
	if (database::removeDuplicatesInToFollow(user) == false) {
		std::cerr << "[-] Error : Unable to remove duplicates" <<
		    std::endl;
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

/*
 * @method      : concatVectors
 * @description : This will take src and add it to dest
 * @input       : destination std::vector, source std::vector
 * @output      : None
 */
template < class T > void concatVectors(std::vector < T > &dest,
					std::vector < T > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}
