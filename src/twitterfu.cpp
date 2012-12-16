#include <iostream>
#include <curl/curl.h>
#include <twitcurl.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <limits>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sys/stat.h>
#include <ctime>
#include "twitterfu.h"

using namespace std;
using boost::property_tree::ptree;

/*
 * @method      : option_select
 * @output      : the options selected
 */
int option_select()
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

/*
 * @method      : config
 * @description : will read the configure file and set up the
 * variables accordingly in the user struct
 * @input       : configuration filename, user struct
 * @output      : true if read successfuly, oterwise false.
 */
bool config(string filename, User * user)
{

	vector < string > vconf = file_to_vector(filename);
	map < string, string > conf;

	for (vector < string >::iterator it = vconf.begin();
	     it != vconf.end(); it++) {
		string token;
		istringstream tokens(*it);
		while (tokens >> token) {
			unsigned pos = token.find('=');
			if (pos != string::npos) {
				conf[token.substr(0, pos)] =
				    token.substr(pos + 1);
			}
		}
	}

	if (conf["username"].empty())
		return false;
	user->username = conf["username"];
	if (conf["password"].empty())
		return false;
	user->password = conf["password"];
	if (conf["consumer_key"].empty())
		return false;
	user->consumer_key = conf["consumer_key"];
	if (conf["consumer_secret"].empty())
		return false;
	user->consumer_secret = conf["consumer_secret"];
	if (conf["to_follow"].empty())
		return false;
	user->cache.to_follow = conf["to_follow"];
	if (conf["followed"].empty())
		return false;
	user->cache.followed = conf["followed"];
	if (conf["unfollowed"].empty())
		return false;
	user->cache.unfollowed = conf["unfollowed"];
	if (!conf["access_token_key"].empty())
		user->access_token_key = conf["access_token_key"];
	if (!conf["access_token_secret"].empty())
		user->access_token_secret = conf["access_token_secret"];
	if (!conf["proxy_username"].empty())
		user->proxy.username = conf["proxy_username"];
	if (!conf["proxy_password"].empty())
		user->proxy.password = conf["proxy_password"];
	if (!conf["proxy_address"].empty())
		user->proxy.address = conf["proxy_address"];
	if (!conf["proxy_port"].empty())
		user->proxy.port = conf["proxy_port"];

	return true;
}

/*
 * @method      : file_to_vector
 * @description : Takes a filename opens it get's the content convert it to a vector and
 * returns the vector.
 * @input       : filename
 * @output      : vector
 *
 */
vector < string > file_to_vector(string filename)
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

	// remove duplicates
	if (remove_duplicates(user) == false) {
		return false;
	}

	vector < string > tofollow(file_to_vector(user->cache.to_follow));
	vector < string > followed(file_to_vector(user->cache.followed));
	vector < string > unfollowed(file_to_vector(user->cache.unfollowed));

	cout << "\tApplication Status :" << endl;
	cout << "\t\tFollowed   : " << followed.size() << endl;
	cout << "\t\tTo follow  : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed : " << unfollowed.size() << endl;

	if (user->twitterObj.accountVerifyCredGet() == true) {
		cout << "\tAccount Status     :" << endl;
		if (parse_lastweb_response(user, "user.followers_count",
					   followers) == false) {
			cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << endl;
			return false;
		}
		if (parse_lastweb_response(user, "user.friends_count",
					   following) == false) {
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
		if (parse_lastweb_response
		    (user, "hash.remaining-hits", remaining_hits) == false) {
			cerr << "[-] Error : Unable to get hash.remaining-hits"
			    << endl;
			return false;
		}

		if (parse_lastweb_response
		    (user, "hash.hourly-limit", hourly_limit) == false) {
			cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		if (parse_lastweb_response(user, "hash.reset-time", reset_time)
		    == false) {
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
 * @method           : create_cache
 * @description      : this is to be run at the beginning, to create
 * the catch files.
 * @input            : user
 * @output           : true if created the files, otherwise false.
 */
bool create_cache(User * user)
{
	if (!file_exists(user->cache.to_follow)) {
		fstream fs(user->cache.to_follow.c_str(), fstream::out);
		if (fs.is_open() == false)
			return false;

	}

	if (!file_exists(user->cache.followed)) {
		fstream fs(user->cache.followed.c_str(), fstream::out);
		if (fs.is_open() == false)
			return false;
	}

	if (!file_exists(user->cache.unfollowed)) {
		fstream fs(user->cache.unfollowed.c_str(), fstream::out);
		if (fs.is_open() == false)
			return false;
	}

	return true;
}

/*
 * @method           : file_exists
 * @description      : check if file exists
 * @input            : filename
 * @output           : true if file exists, false otherwise
 */
bool file_exists(string filename)
{
	struct stat fi;
	return stat(filename.c_str(), &fi) == 0;
}

/*
 * @method           : parse_lastweb_response
 * @description      : parse the last web response and get a
 * value of a node, save the value in T v
 * @input            : user, the node to get, T v
 * @output           : true if got the result false otherwise
 */
template < class T > bool
parse_lastweb_response(User * user, string node, T & v)
{
	string replyMsg = "";
	ptree pt;
	try {
		user->twitterObj.getLastWebResponse(replyMsg);
		stringstream ss(replyMsg);
		read_xml(ss, pt);
		v = pt.get < T > (node.c_str());
	}
	catch(exception const &e) {
		cout << replyMsg << endl;
		return false;
	}
	return true;
}

/*
 * @method      : option_parse
 * @description : Do a specific job depending on the opt
 * @input       : User object, option number
 * @output      : None
 */
void option_parse(User * user, int opt)
{

	string username;

	switch (opt) {
	case 1:		// Get followers of a user
		{
			cout << "Username : ";
			cin >> username;
			if (remove_duplicates(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			if (vector_to_file(user->cache.to_follow,
					   get_followers_of(user,
							    username)) ==
			    false) {
				cerr << "[-] Error : Unable to append vector" <<
				    endl;
				return;
			}
		}
		break;
	case 2:		// follow users
		{
			follow(file_to_vector(user->cache.to_follow), user);
		}
		break;
	case 3:		// our status
		{
			status(user);
		}
		break;
	case 4:		// unfollow users
		{
			unfollow(user);
		}
		break;
	case 5:		// quit
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
	vector < string > followers(get_following_of(user, user->username));
	string replyMsg, who;
	bool isfollow = true;
	long unfollowed = 0;
	gotExitSignal = false;
	fstream fs(user->cache.unfollowed.c_str(), fstream::app | fstream::out);

	if (remove_duplicates(user) == false) {
		cerr << "[-] Error : Unable to remove duplicates" << endl;
		return;
	}
	// check if cache file is opened for appending
	if (fs.is_open() == false) {
		cerr << "[-] Error : Unable to open " << user->
		    cache.unfollowed << endl;
		return;
	}

	/* Install the signal handler */
	if (signal((int)SIGINT, signalhandler) == SIG_ERR) {
		cerr << "\t[-] Error : Unable to install signalHandler" << endl;
		return;
	}
	// Don't do anything if there's no one to unfollow
	if (followers.size() == 0) {
		cerr << "\t[-] Error : No one to unfollow" << endl;
		return;
	}

	/*
	 * Now decide the followers who haven't followed me back
	 * and unfollow them
	 * */
	for (vector < string >::iterator it = followers.begin();
	     it != followers.end() && gotExitSignal != true; it++) {
		user->twitterObj.friendshipShow(*it, true);
		if (parse_lastweb_response(user,
					   "relationship.source.followed_by",
					   isfollow) == false) {
			cerr <<
			    "[-] Error : Unable to find relationship.source.followed_by"
			    << endl;
			break;
		}

		if (isfollow == false) {
			if (parse_lastweb_response(user,
						   "relationship.target.screen_name",
						   who) == false) {
				cerr <<
				    "[-] Error : Unable to find relationship.target.screen_name"
				    << endl;
				break;
			}
			cout << "\t@" << who << " is not following you; ";
			if (user->twitterObj.friendshipDestroy(*it, true)) {
				cout << "[Unfollowed]" << endl;
				unfollowed++;
				fs << *it << endl;	// write to cache
				sleep(randomize(1, 1));
			} else {
				cout << "[Err:Unable to unfollow]" << endl;
			}
		}
		isfollow = true;
	}

	// restart the exit signal flag
	gotExitSignal = false;
	fs.close();
	cout << "\tWe have unfollowed " << unfollowed << "/" << followers.size()
	    << endl;

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

/* @method      : option_show
 * @description : Show available option to the user
 * @input       : None
 * @output      : None
 */
void option_show()
{
	cout << "1) Get followers of a user" << endl;
	cout << "2) Start following" << endl;
	cout << "3) Status" << endl;
	cout << "4) Unfollow users who haven't followed" << endl;
	cout << "5) Quit" << endl;
}

/*
 * @method      : authenticate
 * @description : authenticate the user
 * @input       : user
 * @output      : true if successful, otherwise false
 */
bool authenticate(User * user)
{
	// set twitter user, pass, and consumer {key,secret}
	user->twitterObj.setTwitterUsername(user->username);
	user->twitterObj.setTwitterPassword(user->password);
	user->twitterObj.getOAuth().setConsumerKey(user->consumer_key);
	user->twitterObj.getOAuth().setConsumerSecret(user->consumer_secret);

	// if we already have oauth keys
	if (user->access_token_key.size() && user->access_token_secret.size()) {
		user->twitterObj.getOAuth().
		    setOAuthTokenKey(user->access_token_key);
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

		// save the keys to twitter.conf
		user->twitterObj.getOAuth().
		    getOAuthTokenKey(user->access_token_key);
		user->twitterObj.getOAuth().
		    getOAuthTokenSecret(user->access_token_secret);
		fstream fs("twitter.conf", fstream::app | fstream::out);
		if (fs.is_open() == false) {
			return false;
		}
		fs << "access_token_key=" << user->access_token_key << endl;
		fs << "access_token_secret=" << user->access_token_secret <<
		    endl;
		fs.close();
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

	if (config("./twitter.conf", user) == false) {
		cerr << "[-] Error : while reading configuration file" << endl;
		return -1;
	}

	/* Create cache files */
	if (create_cache(user) == false) {
		cerr << "[-] Error : Unable to create cache files" << endl;
		return -1;
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
		if (parse_lastweb_response(user, "user.friends_count",
					   user->following) == false) {
			cerr << "[-] Error : Unable to find user.friends_count"
			    << endl;
			return -1;
		}

		if (parse_lastweb_response(user, "user.followers_count",
					   user->followers) == false) {
			cerr <<
			    "[-] Error : Unable to find user.followers_count" <<
			    endl;
			return -1;
		}
	} else {
		cerr << "\t[-] Error : Unable to authenticate." << endl;
		return -1;
	}
	cout << "=====================" << endl;
	cout << "Hello @" << user->username << endl;
	cout << "Following : " << user->following << endl;
	cout << "Followers : " << user->followers << endl;
	cout << "=====================" << endl << endl;

	// Before entering the main loop fix the databases
	if (remove_duplicates(user) == false) {
		cerr << "[-] Error : Unable to remove duplicates" << endl;
		return -1;
	}

	/*
	 * Start the loop and do things in here 
	 * signals will be used to avoid some shitty cases
	 **/
	int opt = 0;
	while (opt != 5) {
		option_show();
		opt = option_select();
		option_parse(user, opt);
	}

	return 0;
}

/*
 * @method      : signalhandler
 * @description : Handler for follow/unfollow to set a exitFlag
 * @input       : n the catched signal number
 * @outpu       : None
 */
void signalhandler(int n)
{
	gotExitSignal = true;
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

	if (to_follow.size() == 0) {
		cerr << "\t[-] Error : Please add users to follow" << endl;
		return;
	}
	// remove duplicates
	if (remove_duplicates(user) == false) {
		cerr << "[-] Error : Unable to remove duplicates" << endl;
		return;
	}

	cout << to_follow.size() << " to follow" << endl;

	/* Install the signal handler */
	if (signal((int)SIGINT, signalhandler) == SIG_ERR) {
		cerr << "\t[-] Error : Unable to install signalHandler" << endl;
		return;
	}

	/* Start following things */
	for (vector < string >::iterator it = to_follow.begin();
	     it != to_follow.end() && gotExitSignal != true; it++) {
		if (user->twitterObj.friendshipCreate(*it, true) == true) {
			followed.push_back(*it);
			if (parse_lastweb_response(user, "user.name", username)
			    == false) {
				if (parse_lastweb_response
				    (user, "hash.error", error) == true) {
					cerr << "\t[-] Error : " << error <<
					    endl;
					followed.erase(followed.end());
				} else {
					cerr <<
					    "\t[-] Error : Unable to find user.name"
					    << endl;
				}
				break;
			} else {
				cout << "\tFollowed @" << username << endl;
			}
			// sleep for 1-10 seconds
			sleep(randomize(5, 10));
		}
	}

	/* when signal is caught or when block is over */
	gotExitSignal = false;
	if (followed.size() != 0)
		cout << "\tWe have followed " << followed.size() << "/" <<
		    to_follow.size() << endl;
	if (vector_to_file(user->cache.followed, followed) == false) {
		cerr << "[-] Error : Unable to append vector" << endl;
		return;
	}

}

/*
 * @method      : concat_vectors
 * @description : This will take src and add it to dest
 * @input       : destination vector, source vector
 * @output      : None
 */
template < class T > void concat_vectors(vector < T > &dest, vector < T > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

/*
 * @method      : remove_duplicates
 * @descrption  :
 * This will remove duplicates from the caches
 * 1) it will read the cache content to a vector
 * 2) it will sort and remove uniques from the vector
 * 3) it will delete cache, rewrite the new vector to it.
 * @input       : user 
 * @output      : true if duplicates are removed, otherwise false
 */
bool remove_duplicates(User * user)
{
	vector < string > v_tofollow(file_to_vector(user->cache.to_follow));
	vector < string > v_followed(file_to_vector(user->cache.followed));
	vector < string > v_unfollowed(file_to_vector(user->cache.unfollowed));

	// Unique Sort and fix vector to follow
	sort(v_tofollow.begin(), v_tofollow.end());
	v_tofollow.erase(unique(v_tofollow.begin(), v_tofollow.end()),
			 v_tofollow.end());
	// Unique Sort and fix vector followed 
	sort(v_followed.begin(), v_followed.end());
	v_followed.erase(unique(v_followed.begin(), v_followed.end()),
			 v_followed.end());
	// Unique sort and fix vector of unfollowed
	sort(v_unfollowed.begin(), v_unfollowed.end());
	v_unfollowed.erase(unique(v_unfollowed.begin(), v_unfollowed.end()),
			   v_unfollowed.end());

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

	// Now write the new to follow to a file
	fstream fs(user->cache.to_follow.c_str(), fstream::out);
	if (fs.is_open() == false) {
		return false;
	}
	for (vector < string >::iterator x = v_tofollow.begin();
	     x != v_tofollow.end(); x++) {
		fs << *x << endl;
	}

	return true;
}

/*
 * @method      : vector_to_file
 * @description : takes a vector of string and append it ot a file
 * @input       : filename to append to, vector
 * @output      : false if failed to append
 */
template < class T > bool vector_to_file(string filename, vector < T > v)
{
	fstream fs;
	fs.open(filename.c_str(), fstream::app | fstream::out | fstream::in);

	if (fs.is_open() == false) {
		return false;
	}

	for (vector < string >::iterator it = v.begin(); it != v.end(); it++) {
		fs << *it << endl;
	}

	fs.close();
	return true;
}

/*
 * @method      : get_following_of
 * @description : Fetch the following of a user
 * and create a vector of their userids and return that vector
 * @input       : user, username
 * @output      : vector of userIDs of the username's following.
 *
 * Doesn't support next_cursor
 */
vector < string > get_following_of(User * user, string username)
{
	string replyMsg;
	vector < string > ids;
	cout << "\tGetting following of @" << username << endl;

	if (user->twitterObj.friendsIdsGet(username, false) == true) {
		user->twitterObj.getLastWebResponse(replyMsg);
		ptree pt;
		stringstream ss(replyMsg);
		read_xml(ss, pt);
		try {
			BOOST_FOREACH(ptree::value_type & v,
				      pt.get_child("id_list.ids"))
			    ids.push_back(v.second.data());
		}
		catch(exception const &e) {
			cerr << "\t[-] Error : Invalid user" << endl;
			return ids;
		}
	} else {
		cerr << "\t[-] Error : Failed to get following of @" << username
		    << endl;
		return ids;
	}

	return ids;
}

/*
 * @method      : get_followers_of
 * @description : Get the followers of a user
 * and create a vector of their userIDs and return that vector
 * @input       : user, username
 * @output      : vector of userIDs of the username's followers
 *
 * Doesn't support next_cursor 
 */
vector < string > get_followers_of(User * user, string username)
{
	string replyMsg;
	vector < string > ids;
	ptree pt;

	cout << "\tGetting followers of @" << username << endl;

	// Get the first 100 ids of username and push_back to ids
	if (user->twitterObj.followersIdsGet(username, false) == true) {
		user->twitterObj.getLastWebResponse(replyMsg);
		stringstream ss(replyMsg);
		read_xml(ss, pt);

		try {
			BOOST_FOREACH(ptree::value_type & v,
				      pt.get_child("id_list.ids"))
			    ids.push_back(v.second.data());
		}
		catch(exception const &e) {
			cerr << "\t[-] Error : Invalid user" << endl;
			return ids;
		}
	} else {
		cerr << "\t[-] Error : Failed to get friendsIdsGet from @" <<
		    username << endl;
	}

	cout << "\tWe have " << ids.size() << " new to follow" << endl;
	return ids;
}
