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
bool
status(twitCurl & twitterObj, string f_tofollow, string f_followed,
       string f_unfollowed)
{
	string replyMsg;
	vector < string > tofollow(file_to_vector(f_tofollow));
	vector < string > followed(file_to_vector(f_followed));
	vector < string > unfollowed(file_to_vector(f_unfollowed));

	cout << "\tApplication Status :" << endl;
	cout << "\t\tFollowed   : " << followed.size() << endl;
	cout << "\t\tTo follow  : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed : " << unfollowed.size() << endl << endl;

	cout << "\tAccount Status     :" << endl;
	if (twitterObj.accountVerifyCredGet() == true) {
		string followers, following;
		parse_lastweb_response(twitterObj, "user.followers_count",
				       followers);
		parse_lastweb_response(twitterObj, "user.friends_count",
				       following);
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
bool create_cache(string f_tofollow, string f_followed, string f_unfollowed)
{
	if (!file_exists(f_tofollow)) {
		fstream fs(f_tofollow.c_str(), fstream::out);
		if (fs.is_open() == false)
			return false;

	}

	if (!file_exists(f_followed)) {
		fstream fs(f_followed.c_str(), fstream::out);
		if (fs.is_open() == false)
			return false;
	}

	if (!file_exists(f_unfollowed)) {
		fstream fs(f_unfollowed.c_str(), fstream::out);
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
template < class T > void
parse_lastweb_response(twitCurl & twitterObj, string node, T & v)
{
	string replyMsg = "";
	twitterObj.getLastWebResponse(replyMsg);
	ptree pt;
	stringstream ss(replyMsg);
	read_xml(ss, pt);
	try {
		v = pt.get < T > (node.c_str());
	} catch(exception const &e) {
		cerr << "\t[-] Error : " << e.what() << endl;
		//cout << replyMsg << endl;
		return;
	}
}

/*
 * @method      : parse options
 * @description : Do a specific job depending on the opt
 * @input       : User object, twitterObj, and option number
 * @output      : None
 */
void option_parse(User * user, twitCurl & twitterObj, int opt)
{

	string username;

	switch (opt) {
	case 1:
		{
			cout << "Username : ";
			cin >> username;
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
			vector_to_file("cache/to_follow.txt",
				       get_followers_of(twitterObj, username));
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
		}
		break;
	case 2:
		{
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
			follow(twitterObj,
			       file_to_vector("cache/to_follow.txt"));
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
		}
		break;
	case 3:
		{
			status(twitterObj, "cache/to_follow.txt",
			       "cache/followed.txt", "cache/unfollowed.txt");
		}
		break;
	case 4:
		{
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
			unfollow(twitterObj, "cache/unfollowed.txt",
				 user->username);
			remove_duplicates("cache/to_follow.txt",
					  "cache/followed.txt",
					  "cache/unfollowed.txt");
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
 *
 */
void unfollow(twitCurl & twitterObj, string filename, string username)
{
	vector < string > followers(get_following_of(twitterObj, username));
	string replyMsg;
	string who;
	bool isfollow = true;
	long unfollowed = 0;
	fstream fs(filename.c_str(), fstream::app | fstream::out);

	// check if cache file is opened for appending
	if (fs.is_open() == false) {
		cerr << "[-] Error : Unable to open " << filename << endl;
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
		cout << "\t[-] Error : No one to unfollow" << endl;
		return;
	}

	/*
	 * Now decide the followers who haven't followed me back
	 * and unfollow him
	 * */
	for (vector < string >::iterator it = followers.begin();
	     it != followers.end() && gotExitSignal != true; it++) {
		twitterObj.friendshipShow(*it, true);
		parse_lastweb_response(twitterObj,
				       "relationship.source.followed_by",
				       isfollow);
		if (isfollow == false) {
			// always assume is follow is true in case
			// shit happens we don't unfollow crazily
			isfollow = true;
			parse_lastweb_response(twitterObj,
					       "relationship.target.screen_name",
					       who);
			cout << "\t@" << who << " is not following you; ";
			if (twitterObj.friendshipDestroy(*it, true)) {
				cout << "[Unfollowed]" << endl;
				unfollowed++;
				fs << *it << endl;	// write to cache
				sleep(randomize(1, 10));
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

	create_cache("cache/to_follow.txt", "cache/followed.txt",
		     "cache/unfollowed.txt");

	if (config("./twitter.conf", user) == false) {
		cerr << "[-] Error : while reading configuration file" << endl;
		return -1;
	}

	twitCurl twitterObj;
	string replyMsg;

	/* Set twitter username and password */
	twitterObj.setTwitterUsername(user->username);
	twitterObj.setTwitterPassword(user->password);

	/* Now authenticating */
	twitterObj.getOAuth().setConsumerKey(user->consumer_key);
	twitterObj.getOAuth().setConsumerSecret(user->consumer_secret);
	twitterObj.getOAuth().setOAuthTokenKey(user->access_token_key);
	twitterObj.getOAuth().setOAuthTokenSecret(user->access_token_secret);
	twitterObj.getOAuth().setOAuthScreenName(user->username);

	/* Verifying authentication */
	if (twitterObj.accountVerifyCredGet() == true) {
		parse_lastweb_response(twitterObj, "user.friends_count",
				       user->following);
		parse_lastweb_response(twitterObj, "user.followers_count",
				       user->followers);
	} else {
		cout << "\t[-] Error : Unable to authenticate." << endl;
		return -1;
	}
	cout << "=====================" << endl;
	cout << "Hello @" << user->username << endl;
	cout << "Following : " << user->following << endl;
	cout << "Followers : " << user->followers << endl;
	cout << "=====================" << endl << endl;

	// Before entering the main loop fix the databases
	remove_duplicates("cache/to_follow.txt", "cache/followed.txt",
			  "cache/unfollowed.txt");

	/*
	 * Start the loop and do things in here 
	 * signals will be used to avoid some shitty cases
	 **/
	int opt = 0;
	while (opt != 5) {
		option_show();
		opt = option_select();
		option_parse(user, twitterObj, opt);
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
 * @input       : already authenticated twitterObj, and a vector of user ids to follow
 * @output      : None
 *
 * @todo :
 *      1) Follow only the users we haven't followed, we can do that by
 *      fetching the user's info and check if user.followed ? true:false
 *      if true we follow, if false we pass
 *      2) Any user followed should be removed from the list,
 *      sort the list while at it ? slow process then :( think about it.
 */
void follow(twitCurl & twitterObj, vector < string > to_follow)
{

	if (to_follow.size() == 0) {
		cerr << "\t[-] Error : Please add users to follow" << endl;
		return;
	}

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
		// TODO: support already following msg;
		if (twitterObj.friendshipCreate(*it, true) == true) {
			followed.push_back(*it);
			twitterObj.getLastWebResponse(replyMsg);
			ptree pt;
			stringstream ss(replyMsg);
			read_xml(ss, pt);
			try {
				cout << "\tFollowing " << pt.get < string >
				    ("user.name") << endl;
			}
			catch(exception const &e) {
				//cout << "[-] " << e.what() << endl;
				// Break out if reason is we reached limit
				if (pt.get < string >
				    ("hash.error").find
				    ("You are unable to follow more people at this time")
				    != string::npos) {
					cout <<
					    "\t[-] You have reached the limit."
					    << endl;
					// If we reached the limit then we remove the last
					// followed and break outta l00p
					followed.erase(followed.end());
					break;
				} else {
					cout << "\t[-] Reason : " << pt.get <
					    string > ("hash.error") << endl;
				}
			}

			// sleep for 1-10 seconds
			sleep(randomize(1, 10));
		}
	}

	/*
	 * This block and down will be reached at all the needed cases
	 * when user exists CTRL-C or when the process has finished running
	 * We shall append all the followed users to the followed db
	 **/
	gotExitSignal = false;
	cout << "\tWe have followed " << followed.size() << "/" << to_follow.
	    size() << endl;
	vector_to_file("cache/followed.txt", followed);
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
bool
remove_duplicates(string f_to_follow, string f_followed, string f_unfollowed)
{
	vector < string > v_tofollow(file_to_vector(f_to_follow));
	vector < string > v_followed(file_to_vector(f_followed));
	vector < string > v_unfollowed(file_to_vector(f_unfollowed));

	// if nothing in tofollow or followed then there's nothing
	// to remove no duplicates
	//if (v_tofollow.size() == 0 || v_followed.size() == 0)
	//      return false;

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
	fstream fs(f_to_follow.c_str(), fstream::out);
	if (fs.is_open() == false) {
		cerr << "\t[-] Error : Unable to write a new " << f_to_follow <<
		    endl;
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
//        cout << "Appending vector to " << filename << endl;
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
vector < string > get_following_of(twitCurl & twitterObj, string username)
{
	string replyMsg;
	vector < string > ids;
	cout << "\tGetting following of @" << username << endl;

	if (twitterObj.friendsIdsGet(username, false) == true) {
		twitterObj.getLastWebResponse(replyMsg);
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
vector < string > get_followers_of(twitCurl & twitterObj, string username)
{
	string replyMsg;
	vector < string > ids;
	cout << "\tGetting followers of @" << username << endl;

	// Get the first 100 ids of username and push_back to ids
	if (twitterObj.followersIdsGet(username, false) == true) {
		twitterObj.getLastWebResponse(replyMsg);
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
