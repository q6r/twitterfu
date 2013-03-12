#include "User.h"

bool User::gotExitSignal = false;

User::User() {
        filters = new Filters(this);
        proxy = new Proxy(this);
}

User::~User() {
        delete filters;
        delete proxy;
}

void User::setUsername(string n) {
        username = n;
}

void User::setPassword(string n) {
        password = n;
}

void User::setConsumerKey(string n) {
        consumer_key = n;
}

void User::setConsumerSecret(string n) {
        consumer_secret = n;
}

void User::setAccessTokenKey(string n) {
        access_token_key = n;
}

void User::setAccessTokenSecret(string n) {
        access_token_secret = n;
}

void User::setDBname(string n) {
        db_name = n;
}

void User::setTimezone(string n) {
        timezone = n;
}

void User::setFollowers(string n) {
        followers = n;
}

void User::setFollowing(string n) {
        following = n;        
}

string & User::getUsername() {
        return username;
}

string & User::getPassword() {
        return password;
}

string & User::getConsumerKey() {
        return consumer_key;
}

string & User::getConsumerSecret() {
        return consumer_secret;
}

string & User::getAccessTokenKey() {
        return access_token_key;
}

string & User::getAccessTokenSecret() {
        return access_token_secret;
}

string & User::getDBname() {
        return db_name;
}

string & User::getTimezone() {
        return timezone;
}

string & User::getFollowers() {
        return followers;
}

string & User::getFollowing() {
        return following;
}

bool User::verify() {
        return User::twitterObj.accountVerifyCredGet();
}

bool User::lastResponse(string node, string & v)
{
	string result = "";
	boost::property_tree::ptree pt;
	User::twitterObj.getLastWebResponse(result);
	stringstream ss(result);

	try {
		read_xml(ss, pt);
		v = pt.get < string > (node.c_str());
	}
	catch(exception const &e) {
		return false;
	}

	return true;
}

void User::signalHandler(int n)
{
	User::gotExitSignal = true;
}

vector < string > User::search(string query)
{
	string jsonResult;
	boost::property_tree::ptree pt;
	vector < string > ids;

	// replace all spaces with %20
	for (size_t pos = query.find(' '); pos != string::npos;
	     pos = query.find(' ', pos))
		query.replace(pos, 1, "%20");

	// send the search query
	if (User::twitterObj.search(query) == false)
		return ids;

	// Get results, parse then and push to ids vector
	User::twitterObj.getLastWebResponse(jsonResult);
	stringstream ss(jsonResult);

	try {
		read_json(ss, pt);
		BOOST_FOREACH(const boost::property_tree::ptree::
			      value_type & child, pt.get_child("results")) {
			ids.push_back(child.second.get < string >
				      ("from_user_id"));
		}
	} catch(exception const &e) {
		cerr << "(Err:Unable to parse json)" << endl;
		return ids;
	}

	return ids;
}

vector < string > User::getFollowing(string username)
{
	string result, err, next_cursor;
	vector < string > ids;
	next_cursor = "-1";

	cout << "[+] Getting following of @" << username << endl;

	do {
		if (User::twitterObj.friendsIdsGet(next_cursor, username,
						   false) == true) {
			User::twitterObj.getLastWebResponse(result);
			boost::property_tree::ptree pt;
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched and error ? */
			if (User::lastResponse("hash.error", err) == true) {
				cerr << "[-] " << err << endl;
				return ids;
			}

			/* Get next cursor */
			User::lastResponse("id_list.next_cursor", next_cursor);

			try {
				BOOST_FOREACH(boost::property_tree::ptree::
					      value_type & v,
					      pt.get_child("id_list.ids"))
				    ids.push_back(v.second.data());
			}
			catch(exception const &e) {
				
				    cerr << "[-] GetFollowingOf Exception" <<
				    endl;
				return ids;
			}
		} else {
			cerr << "[-] Failed to get following of @" <<
			    username << endl;
			return ids;
		}
	}
	while (next_cursor != "0");

	return ids;
}

vector < string > User::getFollowers(string username)
{
	string result, err, next_cursor;
	vector < string > ids;
	boost::property_tree::ptree pt;

	next_cursor = "-1";

	cout << "[+] Getting followers of @" << username << endl;

	do {
		if (User::twitterObj.followersIdsGet(next_cursor, username,
						     false) == true) {
			User::twitterObj.getLastWebResponse(result);
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched error ? */
			if (User::lastResponse("hash.error", err) == true) {
				cerr << "[-] " << err << endl;
				return ids;
			}

			/* Get next cursor */
			User::lastResponse("id_list.next_cursor", next_cursor);

			try {
				BOOST_FOREACH(boost::property_tree::ptree::
					      value_type & v,
					      pt.get_child("id_list.ids"))
				    ids.push_back(v.second.data());
			}
			catch(exception const &e) {
				
				    cerr << "[-] getFollowersOf Exception" <<
				    endl;
				return ids;
			}
		} else {
			cerr <<
			    "[-] Failed to get friendsIdsGet from @" << username
			    << endl;
		}
	}
	while (next_cursor != "0");

	return ids;
}

bool User::authenticate()
{
	string q, authurl, pin;

	// set twitter user, pass, and consumer {key,secret}
        User::twitterObj.setTwitterUsername(User::getUsername());
	User::twitterObj.setTwitterPassword(User::getPassword());
	User::twitterObj.getOAuth().setConsumerKey(User::getConsumerKey());
	User::twitterObj.getOAuth().setConsumerSecret(User::getConsumerSecret());

	// if we already have oauth keys
	if (User::getAccessTokenKey().size() && User::getAccessTokenSecret().size()) {
		User::twitterObj.getOAuth().setOAuthTokenKey(User::getAccessTokenKey());
		User::twitterObj.getOAuth().
		    setOAuthTokenSecret(User::getAccessTokenSecret());
		return true;
	} else {		// if we don't
		// get pin
		if(User::twitterObj.oAuthRequestToken(authurl) == false) {
                        cerr << "[-] Failed while trying to get auth url" << endl;
                        return false;
                }
		cout <<
		    "Visit twitter and authorize the application then enter the PIN."
		    << endl << authurl << endl;
		cout << "PIN : ";
		cin >> pin;
		User::twitterObj.getOAuth().setOAuthPin(pin);
		User::twitterObj.oAuthAccessToken();

		// update database with access keys
		User::twitterObj.getOAuth().getOAuthTokenKey(User::getAccessTokenKey());
		User::twitterObj.getOAuth().
		    getOAuthTokenSecret(User::getAccessTokenSecret());

		q = "UPDATE Config SET access_key = \"" +
		    User::getAccessTokenKey() + "\" WHERE Id=1;";
		if (User::db.execute(q.c_str()) != 0)
			return false;
		q = "UPDATE Config SET access_secret = \"" +
		    User::getAccessTokenSecret() + "\" WHERE Id=1;";
		if (User::db.execute(q.c_str()) != 0)
			return false;
		return true;
	}

	return false;
}


void User::follow(vector < string > to_follow)
{
	User::gotExitSignal = false;
	string username, error, result;
	vector < string > followed;
	vector < string >::iterator it;
	int ignored = 0;

	if (to_follow.size() == 0) {
		cerr << "(Err:Please add users to follow)" << endl;
		return;
	}
	// remove duplicates
	if (removeDuplicatesInToFollow(this) == false) {
		cerr << "(Err:Unable to remove duplicates)" << endl;
		return;
	}

	cout << to_follow.size() << " to follow" << endl;

	/* Install the signal handler */
	if (signal((int)SIGINT, User::signalHandler) == SIG_ERR) {
		cerr << "(Err:Unable to install signalHandler)" << 
		    endl;
		return;
	}

	for (it = to_follow.begin(); it != to_follow.end() && User::gotExitSignal != true; it++) {	// { Users to follow

		// follow only those that applies to the
		// by_ratio filter
		if (User::filters->mainFilter(*it) == true) {
			if (User::twitterObj.friendshipCreate(*it, true) == true) {	// if followed the user
				followed.push_back(*it);
				if (User::lastResponse("user.name", username) == false) {	// if can't get username
					if (User::lastResponse("hash.error", error) == true) {	// get hash error

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
							
							    cerr << "(Err:" <<
							    error << ")";
							cleanLine(120);
							followed.push_back(*it);	// We have followed the user
						}
					}
				} else {	// user followed
					cout << "Followed " << username;
					cleanLine(120);
				}
			} else {	// unable to create friendship
				cerr << "(Err:Unable to follow)";
				cleanLine(120);
			}
			sleep(randomize(3, 5));
		} else {	// filter ignored someone
			// Did we reach API limit?
			if (User::lastResponse("hash.error", error) == true) {
				if (string::npos !=
				    error.find("Clients may not make")) {
					
					    cerr <<
					    "The client have reached the API limit, please try again in an hour"
					    << endl;
					break;
				}
			}
			// Who did we ignore ?
			if (User::lastResponse("user.name", username) == true) {
				cout << "Ignored " << username;
				cleanLine(120);
			}
			ignored++;
		}
	}			// } users to follow

	/* when signal is caught or when block is over */
	User::gotExitSignal = false;
	if (followed.size() != 0)
		cout << endl << "We have followed " << followed.
		    size() << "/" << to_follow.size() - ignored << endl;

	if (toDB(this, followed, "Followed", "userid") == false) {
		cerr << "[-] Error : toDB" << endl;
		return;
	}
	if (ignored > 0)
		cout << "\tWe have Ignored : " << ignored << endl;
}

bool User::configure()
{
	string address, port, username, password, q;
	int opt = -1;

	cout << "1) Set proxy" << endl;
	cout << "2) Filters" << endl;
	cout << "3) Purge To Follow" << endl;
	cout << "4) Purge Followed" << endl;
	cout << "5) Purge Unfollowed" << endl;
	cout << "6) Purge MyFollowers" << endl;
	cout << "7) Pruge all" << endl;
	cout << "8) Return" << endl;

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

			if (this->proxy->change_proxy
			    (address, port, username, password) == false)
				
				    cerr << "[-] Error Unable to change proxy"
				    << endl;
		}
		break;
	case 2:		// filters
		{
                        User::filters->filterList();
		}
		break;
	case 3:		// purge to follow
		{
			if (purgeTable(this, "ToFollow") == false)
				
				    cerr <<
				    "[-] Error : Unable toi purge ToFollow" <<
				    endl;
		}
		break;
	case 4:		// purge followed
		{
			if (purgeTable(this, "Followed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    endl;
		}
		break;
	case 5:		// purge unfollowed
		{
			if (purgeTable(this, "UnFollowed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge UnFollowed" <<
				    endl;
		}
		break;
	case 6:
		{
			if (purgeTable(this, "MyFollowers") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    endl;
		}
		break;
	case 7:		// purge all
		{
			if (purgeTable(this, "ToFollow") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge ToFollow" <<
				    endl;

			if (purgeTable(this, "Followed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    endl;

			if (purgeTable(this, "UnFollowed") == false)
				
				    cerr << "[-] Error : Unable to purge " <<
				    endl;

			if (purgeTable(this, "MyFollowers") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    endl;
		}
		break;
	case 8:		// return
		break;
	default:
		break;
	}


	return true;
}

void User::unfollow()
{
	vector < string >
	    followers(getVal(this, "MyFollowers", "userid"));
	vector < string >::iterator it;
	vector < string > result;
	string who;
	string isfollow = "true";
	long unfollowed = 0;
	int remainingHits = 0;
	User::gotExitSignal = false;

	// Don't do anything if there's no one to unfollow
	if (followers.size() == 0) {
		cerr << "(Err:No one to unfollow)" << endl;
		return;
	}
	// Install the signal handler
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		cerr << "(Err:Unable to install signalHandler)" << 
		    endl;
		return;
	}

	/*
	 * Now decide the followers who haven't followed me back
	 * and unfollow them
	 * */
	for (it = followers.begin();
	     it != followers.end() && gotExitSignal != true; it++) {

		User::twitterObj.friendshipShow(*it, true);

		/* get followed_by and if fails check
		 * the remaining hits and reconfigure
		 **/
		if (User::lastResponse(
				 "relationship.source.followed_by",
				 isfollow) == false) {
			// check if we reached the limit
			remainingHits = User::getRemainingHits();
			if (remainingHits == 0) {
				cerr <<
				    "[-] Error : You have reached the limit how about using a proxy ?"
				    << endl;
				if (User::configure() == false) {
					
					    cerr <<
					    "[-] Error : Unable to configure" <<
					    endl;
				} else {
					
					    cout << "Rerun to apply changes" <<
					    endl;
				}
			} else {	// unknown exception
				cerr <<
				    "(Err: Unable to find relationship.source.followed_by)"
				    << endl;
			}
			break;
		}

		/* If the user is not following us */
		if (isfollow == "false") {
			if (User::lastResponse(
					 "relationship.target.screen_name",
					 who) == false) {
				
				    cerr <<
				    "(Err:Unable to find relationship.target.screen_name)"
				    << endl;
				break;
			}
			if (User::twitterObj.friendshipDestroy(*it, true)) {
				cout << "Unfollowed " << who;
				unfollowed++;
				result.push_back(*it);
				cleanLine(120);
				sleep(randomize(3, 5));
			} else {
				cout << "Unable to Unfollow " << who;
				cleanLine(120);
			}
		}

		isfollow = "true";
	}

	// restart the exit signal flag
	gotExitSignal = false;
	cout << "We have unfollowed " << unfollowed << "/" <<
	    followers.size() << endl;

	// write results to db
	if (toDB(this, result, "UnFollowed", "userid") == false)
		cerr << "[-] Error : Unable to write to db" << endl;
}


bool User::status()
{
	string result, followers, following, reset_time;
	string remaining_hits, hourly_limit;

	vector < string >
	    tofollow(toVector(this, "ToFollow", "userid"));
	vector < string >
	    followed(toVector(this, "Followed", "userid"));
	vector < string >
	    unfollowed(toVector(this, "UnFollowed", "userid"));
	vector < string >
	    myfollowers(toVector(this, "MyFollowers", "userid"));

	cout << "\tDatabase Status :" << endl;
	cout << "\t\tFollowed     : " << followed.size() << endl;
	cout << "\t\tTo follow    : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed   : " << unfollowed.size() << endl;
	cout << "\t\tMy followers : " << myfollowers.size() << endl;

	if (User::twitterObj.accountVerifyCredGet() == true) {
		cout << "\tAccount Status     :" << endl;
		if (User::lastResponse("user.followers_count", followers) ==
		    false) {
			
			    cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << endl;
			return false;
		}
		if (User::lastResponse("user.friends_count", following) ==
		    false) {
			
			    cerr << "\t[-] Error : Unable to find friends_count"
			    << endl;
			return false;
		}

		cout << "\t\tFollowers  : " << followers << endl;
		cout << "\t\tFollowing  : " << following << endl;
	} else {
		
		    cerr << "\t[-] Error : Unable to get account status." <<
		    endl;
		return false;
	}

	// API status
	if (User::twitterObj.accountRateLimitGet() == true) {


		cout << "\tAPI Status : " << endl;
		if (User::lastResponse("hash.remaining-hits", remaining_hits) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.remaining-hits" <<
			    endl;
			return false;
		}

		if (User::lastResponse("hash.hourly-limit", hourly_limit) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		if (User::lastResponse("hash.reset-time", reset_time) == false) {
			cerr <<
			    "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		cout << "\t\tRemaining hits : " << remaining_hits << 
		    endl;
		cout << "\t\tHourly limit   : " << hourly_limit << 
		    endl;
		cout << "\t\tReset  at      : " << reset_time << endl;
	} else {
		cerr << "\t[-] Error : Unable to get API status" << 
		    endl;
		return false;
	}

	return true;
}

int User::getRemainingHits()
{
	string temp_limit;
	int limit;

	// Get account limits remaining hits
	if (User::twitterObj.accountRateLimitGet() == true) {
		if (User::lastResponse("hash.remaining-hits", temp_limit) ==
		    true) {
			stringstream ss(temp_limit);
			ss >> limit;
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	return limit;
};
