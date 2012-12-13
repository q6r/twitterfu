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
 * @method      : option select
 * @output      : the options selected
 */
int option_select()
{
	int opt = -1;
	cout << "> ";
	cin >> opt;
	// Avoid infinite loop
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
	if (conf["access_key"].empty())
		return false;
	user->access_token_key = conf["access_key"];
	if (conf["access_secret"].empty())
		return false;
	user->access_token_secret = conf["access_secret"];

	if (conf["to_follow"].empty())
		return false;
	user->cache.to_follow = conf["to_follow"];

	if (conf["followed"].empty())
		return false;
	user->cache.followed = conf["followed"];
	if (conf["unfollowed"].empty())
		return false;
	user->cache.unfollowed = conf["unfollowed"];

	return true;
}

/*
 * @method      : file to vector
 * @description : Takes a filename opens it get's the content convert it to a vector and
 * returns the vector.
 * @input       : filename
 * @output      : vector
 *
 */
vector < string > file_to_vector(string filename)
{
	vector < string > v;
	fstream fs(filename.c_str(), fstream::in);
	if (fs.is_open() == false) {
		cerr << "\t[-] Error : Unable to open " << filename << endl;
		return v;
	}

	string temp;
	while (fs.eof() == false) {
		getline(fs, temp);
		if (temp.length() != 0)
			v.push_back(temp);
	}

	fs.close();
	return v;
}

/* @method      : Our status
 * @description : Show how many did we follow, to follow, our followers,
 * and our following.
 * @input       : filename of tofollow cache and followed cache
 * @output      : None
 */
bool status(User * user)
{
	string replyMsg, followers, following;

	// remove duplicates
	remove_duplicates(user);

	vector < string > tofollow(file_to_vector(user->cache.to_follow));
	vector < string > followed(file_to_vector(user->cache.followed));
	vector < string > unfollowed(file_to_vector(user->cache.unfollowed));

	cout << "\tApplication Status :" << endl;
	cout << "\t\tFollowed   : " << followed.size() << endl;
	cout << "\t\tTo follow  : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed : " << unfollowed.size() << endl << endl;

	cout << "\tAccount Status     :" << endl;
	if (user->twitterObj.accountVerifyCredGet() == true) {
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
		cerr << "\t[-] Error : Unable to get last web response." <<
		    endl;
		return false;
	}

	return true;
}

/* @method           : randomize from to
 * @description      : will get a random number from to (int)
 * @input            : from, to
 * @output           : from <= random <= to
 */
int randomize(int from, int to)
{
	return rand() % to + from;
}

/*
 * @method           : create cache files
 * @description      : this is to be run at the beginning, to create
 * the catch files.
 * @input            : names of the files
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
 * @method           : file exists
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
 * @method           : parse last web response
 * @description      : parse the last web response and get a
 * value of a node, save the value in T v
 * @input            : twitter object, the node to get, T v
 * @output           : None
 */
template < class T > bool
parse_lastweb_response(User * user, string node, T & v)
{
	string replyMsg = "";
	try {
		user->twitterObj.getLastWebResponse(replyMsg);
		ptree pt;
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
 * @method      : parse options
 * @description : Do a specific job depending on the opt
 * @input       : User object, user->twitterObj, and option number
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
			remove_duplicates(user);
			vector_to_file(user->cache.to_follow,
				       get_followers_of(user, username));
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
	case 5:
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
 * @method      : Unfollow users
 * @description : Unfollow users who haven't followed me back
 * @input       : twitterobject, filename to output the users we unfollowed,
 * and our username
 * @output      : None
 */
void unfollow(User * user)
{
	vector < string > followers(get_following_of(user, user->username));
	string replyMsg, who;
	bool isfollow = true;
	long unfollowed = 0;
	fstream fs(user->cache.unfollowed.c_str(), fstream::app | fstream::out);

	remove_duplicates(user);

	// check if cache file is opened for appending
	if (fs.is_open() == false) {
		cerr << "[-] Error : Unable to open " << user->cache.
		    unfollowed << endl;
		return;
	}

	/* Install the signal handler */
	gotExitSignal = false;
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
	 * and unfollow him
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
			// always assume is follow is true in case
			// shit happens we don't unfollow crazily
			isfollow = true;
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
				sleep(randomize(5, 10));
			} else {
				cout << "[Err:Unable to unfollow]" << endl;
			}
		}
	}

	// restart the exit signal flag
	// and print result
	gotExitSignal = false;
	fs.close();
	cout << "\tWe have unfollowed " << unfollowed << "/" << followers.size()
	    << endl;

	return;
}

/* @method      : twitcurl destructor
 * @description : This is here because the destructor is not defined
 * in twitCurl library which when called can cause a segmentation fault.
 * @input       : None
 * @output      : None
 */
twitCurl::~twitCurl()
{
	// Do nothing
}

/* @method      : show options
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
	return;
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

	create_cache(user);

	if (config("./twitter.conf", user) == false) {
		cerr << "[-] Error : while reading configuration file" << endl;
		return -1;
	}

	string replyMsg;

	/* Set twitter username and password */
	user->twitterObj.setTwitterUsername(user->username);
	user->twitterObj.setTwitterPassword(user->password);

	/* Now authenticating */
	user->twitterObj.getOAuth().setConsumerKey(user->consumer_key);
	user->twitterObj.getOAuth().setConsumerSecret(user->consumer_secret);
	user->twitterObj.getOAuth().setOAuthTokenKey(user->access_token_key);
	user->twitterObj.getOAuth().setOAuthTokenSecret(user->
							access_token_secret);
	user->twitterObj.getOAuth().setOAuthScreenName(user->username);

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
	remove_duplicates(user);

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
 * @method      : signal handler
 * @description : Handler for follow/unfollow to set a exitFlag
 * @input       : n which is the catched signal number
 * @outpu       : None
 */
void signalhandler(int n)
{
	//cout << "\tGot Exception : " << n << endl;
	gotExitSignal = true;
}

/* @method      : follow()
 * @description : It will follow a vector of user_ids
 * @input       : already authenticated user->twitterObj, and a vector of user ids to follow
 * @output      : None
 *
 * @todo :
 *      1) Follow only the users we haven't followed, we can do that by
 *      fetching the user's info and check if user.followed ? true:false
 *      if true we follow, if false we pass
 *      2) Any user followed should be removed from the list,
 *      sort the list while at it ? slow process then :( think about it.
 */
void follow(vector < string > to_follow, User * user)
{
	string username, error;

	if (to_follow.size() == 0) {
		cerr << "\t[-] Error : Please add users to follow" << endl;
		return;
	}
	// remove duplicates
	remove_duplicates(user);

	cout << to_follow.size() << " to follow" << endl;

	/* Install the signal handler */
	gotExitSignal = false;
	if (signal((int)SIGINT, signalhandler) == SIG_ERR) {
		cerr << "\t[-] Error : Unable to install signalHandler" << endl;
		return;
	}

	/* Start following things */
	vector < string > followed;
	string replyMsg;

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
		cout << "\tWe have followed " << followed.
		    size() << "/" << to_follow.size() << endl;
	vector_to_file(user->cache.followed, followed);
}

/*
 * @method      : concat vector
 * @description : This will take src and add it to dest
 * @input       : destination vector, source vector
 * @output      : None
 *
 * @TODO        : use templates to support all types of vectors
 */
void concat_vectors(vector < string > &dest, vector < string > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

/*
 * @method      : remove duplicates
 * @descrption  : This will take a file (mostly the to-follow list)
 * This will remove duplicates from the file
 * 1) it will read the file content to a vector
 * 2) it will sort and remove uniques from the vector
 * 3) it will delete filename, rewrite the new vector to it.
 *
 * Simply it will fix the tofollow list by removing who ever
 * we have unfollowed before, or followed.. so we don't follow
 * someone who we have followed before, or follow someone who
 * we have unfollowed already. :>
 *
 * @input       : cache of tofollow, followed, unfollowed
 * @output      : false if failed in many cases, true if successful
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
		cerr << "\t[-] Error : Unable to write a new " << user->cache.
		    to_follow << endl;
		return false;
	}
	for (vector < string >::iterator x = v_tofollow.begin();
	     x != v_tofollow.end(); x++) {
		fs << *x << endl;
	}

	return true;
}

/*
 * @method      : vector to file
 * @description : takes a vector of string and append it ot a file
 * @input       : filename to append to, vector
 * @TODO        :
 *      use templates to support all vectors types
 */
bool vector_to_file(string filename, vector < string > v)
{
	fstream fs;
	fs.open(filename.c_str(), fstream::app | fstream::out | fstream::in);

	if (fs.is_open() == false) {
		cerr << "\t[-] Error : unable to append vector to " << filename
		    << endl;
		return false;
	}

	for (vector < string >::iterator it = v.begin(); it != v.end(); it++) {
		fs << *it << endl;
	}

	fs.close();
	return true;
}

/*
 * @method      : Get following of a user (who is he following)
 * @description : Will use a twitter Object to fetch the following of a user
 * and create a vector of their userids and return that vector
 * @input       : twitter object, username
 * @output      : vector of userIDs of the username following.
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
 * @method      : Get the followers of a user (who are following him)
 * @description : Will use a twitter Object to fetch the followers of a user
 * and create a vector of their userIDs and return that vector
 * @input       : twitter object, username
 * @output      : vector of userIDs of the username followers'
 *
 * Doesn't support next_cursor 
 */
vector < string > get_followers_of(User * user, string username)
{
	string replyMsg;
	vector < string > ids;
	cout << "\tGetting followers of @" << username << endl;

	// Get the first 100 ids of username and push_back to ids
	if (user->twitterObj.followersIdsGet(username, false) == true) {
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
		cerr << "\t[-] Error : Failed to get friendsIdsGet from @" <<
		    username << endl;
	}

	cout << "\tWe have " << ids.size() << " new to follow" << endl;
	return ids;
}
