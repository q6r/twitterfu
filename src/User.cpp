#include "User.h"

bool User::gotExitSignal = false;

User::User(string _database, string _consumer_key, string _consumer_secret) {
    User::set("db_name", _database);
    User::set("consumer_key", _consumer_key);
    User::set("consumer_secret", _consumer_secret);
    
    proxy    = new Proxy(this);
    filters  = new Filters(this);
    database = new Database(this);

    proxy->setup(); // set up proxy
    
    /* Authenticate the user and set the flag accordingly */ 
    (User::authenticate()==true) ? User::authenticated=true:User::authenticated=false;
}

bool User::isAuthenticated() {
    return authenticated;
}

bool User::reachedLimit() {
    bool limit = false;
    (User::getRemainingHits()==0) ? limit=true:limit=false;
    return limit;
}

User::~User() {
    delete database;
    delete filters;
    delete proxy;
}

void User::set(string key, string val) {
    conf[key] = val;
}

string & User::get(string key) {
    return conf[key];
}

bool User::verify() {
    bool verified = User::twitterObj.accountVerifyCredGet();
    string temp;

    // If user is verified
    if(verified) {
        // get and set following
        if (User::lastResponse("user.friends_count", temp) == false) {
            cerr << "[-] Error : Unable to find user.friends_count" << endl;
            return -1;
        } else {
            User::set("following", temp );
        }
        // get and set followers
        if (User::lastResponse("user.followers_count",temp) == false) {
            cerr << "[-] Error : Unable to find user.followers_count" << endl;
            return -1;
        } else {
            User::set("followers", temp );
        }
        // get and set timezone if not set
        if (User::get("timezone").empty()) {
            if (User::lastResponse("user.time_zone", temp) == false) {
                cerr << "[-] Error : Unable to find timezone" << endl;
                return -1;
            } else {
                // if there's timezone put it in db
                User::set("timezone", temp );
                User::database->setupTimezone( User::get("timezone") );
            }
        }
    }

    return verified;

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

deque < string > User::search(string query)
{
	string jsonResult;
	boost::property_tree::ptree pt;
	deque < string > ids;

	// replace all spaces with %20
	for (size_t pos = query.find(' '); pos != string::npos;
            pos = query.find(' ', pos))
        query.replace(pos, 1, "%20");

	// send the search query
	if (User::twitterObj.search(query) == false)
		return ids;

	// Get results, parse then and push to ids deque
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

deque < string > User::getFollowing(string username)
{
	string result, err, next_cursor;
	deque < string > ids;
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

deque < string > User::getFollowers(string username)
{
	string result, err, next_cursor;
	deque < string > ids;
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
    User::twitterObj.setTwitterUsername(User::get("username"));
	User::twitterObj.setTwitterPassword(User::get("password"));
	User::twitterObj.getOAuth().setConsumerKey(User::get("consumer_key"));
	User::twitterObj.getOAuth().setConsumerSecret(User::get("consumer_secret"));

	// if we already have oauth keys
	if (User::get("access_token_key").size() && User::get("access_token_secret").size()) {
		User::twitterObj.getOAuth().setOAuthTokenKey(User::get("access_token_key"));
		User::twitterObj.getOAuth().
		    setOAuthTokenSecret(User::get("access_token_secret"));
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
		User::twitterObj.getOAuth().getOAuthTokenKey(User::get("access_token_key"));
		User::twitterObj.getOAuth().getOAuthTokenSecret(User::get("access_token_secret"));

		q = "UPDATE Config SET access_key = \"" +
		    User::get("access_token_key") + "\" WHERE Id=1;";
		if (User::db.execute(q.c_str()) != 0)
			return false;
		q = "UPDATE Config SET access_secret = \"" +
		    User::get("access_token_secret") + "\" WHERE Id=1;";
		if (User::db.execute(q.c_str()) != 0)
			return false;
		return true;
	}

	return false;
}


void User::follow(deque < string > to_follow)
{
	User::gotExitSignal = false;
	string username, error, result;
	deque < string > followed;
	deque < string >::iterator it;
	int ignored = 0;

	if (to_follow.size() == 0) {
		cerr << "(Err:Please add users to follow)" << endl;
		return;
	}
	// remove duplicates
	if (User::database->removeDuplicatesInToFollow() == false) {
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

	if (User::database->toDB(followed, "Followed", "userid") == false) {
		cerr << "[-] Error : toDB" << endl;
		return;
	}
	if (ignored > 0)
		cout << "\tWe have Ignored : " << ignored << endl;
}

int User::randomize(int from, int to)
{
	return rand() % to + from;
}

void User::cleanLine(int n)
{
	for (int i = 0; i < n; i++)
		cout << " ";
	cout << "\xd";
	flush(cout);
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

    cout << "> ";
    cin >> opt;

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
			if (User::database->purgeTable("ToFollow") == false)
				
				    cerr <<
				    "[-] Error : Unable toi purge ToFollow" <<
				    endl;
		}
		break;
	case 4:		// purge followed
		{
			if (User::database->purgeTable("Followed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    endl;
		}
		break;
	case 5:		// purge unfollowed
		{
			if (User::database->purgeTable("UnFollowed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge UnFollowed" <<
				    endl;
		}
		break;
	case 6:
		{
			if (User::database->purgeTable("MyFollowers") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge MyFollowers" <<
				    endl;
		}
		break;
	case 7:		// purge all
		{
			if (User::database->purgeTable("ToFollow") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge ToFollow" <<
				    endl;

			if (User::database->purgeTable("Followed") == false)
				
				    cerr <<
				    "[-] Error : Unable to purge Followed" <<
				    endl;

			if (User::database->purgeTable("UnFollowed") == false)
				
				    cerr << "[-] Error : Unable to purge " <<
				    endl;

			if (User::database->purgeTable("MyFollowers") == false)
				
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
	deque < string > followers(User::database->getVal("MyFollowers", "userid"));
	deque < string >::iterator it;
	deque < string > result;
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
	if (User::database->toDB(result, "UnFollowed", "userid") == false)
		cerr << "[-] Error : Unable to write to db" << endl;
}


bool User::status()
{
	string result, followers, following, reset_time;
	string remaining_hits, hourly_limit;

	deque < string >
	    tofollow(User::database->toVector( "ToFollow", "userid"));
	deque < string >
	    followed(User::database->toVector( "Followed", "userid"));
	deque < string >
	    unfollowed(User::database->toVector( "UnFollowed", "userid"));
	deque < string >
	    myfollowers(User::database->toVector( "MyFollowers", "userid"));

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
