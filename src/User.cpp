#include "User.h"

User::User(const string &_username, const string &_password, const string &_consumer_key, const string &_consumer_secret) :
    authenticated(false)
{

    srand(time(NULL));

    User::set("username", _username);
    User::set("password", _password);
    User::set("consumer_key", _consumer_key);
    User::set("consumer_secret", _consumer_secret);
    
    proxy    = new Proxy(this);
    filters  = new Filters(this);

    proxy->setup(); // set up proxy

    // Get our own following and followers
    my_followers = this->getFollowers( this->get("username") );
    my_following = this->getFollowing( this->get("username") );

    /* Authenticate the user and set the flag accordingly */ 
    (this->authenticate()==true) ? this->authenticated=true: this->authenticated=false;
}

bool User::isAuthenticated() const {
    return this->authenticated;
}

bool User::reachedLimit() {
    bool limit = false;
    (this->getRemainingHits()==0) ? limit=true:limit=false;
    return limit;
}

User::~User() {
    delete filters;
    delete proxy;
}

void User::set(const string &key, const string &val) {
    conf[key] = val;
}

string & User::get(const string &key) {
    return conf[key];
}

bool User::verify() {
    bool verified = User::twitterObj.accountVerifyCredGet();
    string temp;

    // If user is verified
    if(verified) {
        // get and set following
        if (this->lastResponse("user.friends_count", temp) == false)
            return false;
        else
            User::set("following", temp );
        // get and set followers
        if (this->lastResponse("user.followers_count",temp) == false)
            return false;
        else
            User::set("followers", temp );

        // get and set timezone if not set
        if (this->lastResponse("user.time_zone", temp) == false)
            return false;
        else
            User::set("timezone", temp );
    }

    return verified;

}

bool User::lastResponse(string node, string & v)
{
	string result = "";
	boost::property_tree::ptree pt;
	this->twitterObj.getLastWebResponse(result);
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
	if (this->twitterObj.search(query) == false)
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

twitCurl User::getTwitterObj() const {
    return this->twitterObj;
}

deque < string > User::getFollowing(string username)
{
	string result, err, next_cursor;
	deque < string > ids;
	next_cursor = "-1";

	cout << "[+] Getting following of @" << username << endl;

	do {
		if (this->twitterObj.friendsIdsGet(next_cursor, username,
						   false) == true) {
			User::twitterObj.getLastWebResponse(result);
			boost::property_tree::ptree pt;
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched and error ? */
			if (this->lastResponse("hash.error", err) == true) {
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
		if (this->twitterObj.followersIdsGet(next_cursor, username,
						     false) == true) {
			User::twitterObj.getLastWebResponse(result);
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched error ? */
			if (this->lastResponse("hash.error", err) == true) {
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
	string authurl;

	// set twitter user, pass, and consumer {key,secret}
    User::twitterObj.setTwitterUsername(this->get("username"));
	User::twitterObj.setTwitterPassword(this->get("password"));
	User::twitterObj.getOAuth().setConsumerKey(this->get("consumer_key"));
	User::twitterObj.getOAuth().setConsumerSecret(this->get("consumer_secret"));

    // get authentication url
    if(this->twitterObj.oAuthRequestToken(authurl) == false) {
        cerr << "[-] Failed while trying to get auth url" << endl;
        return false;
    }

    // pass this url and handle authentication
    this->twitterObj.oAuthHandlePIN( authurl );
    this->twitterObj.oAuthAccessToken();

    // get access token TODO : remove?
    User::twitterObj.getOAuth().getOAuthTokenKey(this->get("access_token_key"));
    User::twitterObj.getOAuth().getOAuthTokenSecret(this->get("access_token_secret"));

	return true;
}


bool User::follow(string id, string &username)
{
    std::string error;

    // If the user doesn't meet our filter requirements
    if(this->filters->mainFilter(id) == false)
    {
        return false;
    }

    // Create friendship by userid
    if (this->twitterObj.friendshipCreate(id, true) == true) {

        if(this->lastResponse("user.name", username) == false)
            username = "";

        // Error found display and return false;
        if(this->lastResponse("hash.error", error) == true) {
            std::cout << "because : " << error << std::endl;
            return false;
        }


    } else {
        return false;
    }

    // temporary sleep so we don't fuckup
    sleep(randomize(1,2));

    return true;
}

int User::randomize(const int &from, const int &to) const
{
	return rand() % to + from;
}

bool User::unfollow(string id) {
    string isfollow = "true";

    // Check if the id is following/not following us
    this->twitterObj.friendshipShow(id, true);
    if (this->lastResponse("relationship.source.followed_by",isfollow) == false) {
        std::cout << "Can't find relationship.source.followed_by" << std::endl;
        return false;
    }

    /* If the user is not following us */
    if (isfollow == "false") {
        if (this->twitterObj.friendshipDestroy(id, true)) {
            cout << "Unfollowed " << id << std::endl;;
            return true;
        } else {
            cout << "Unable to Unfollow " << id << std::endl;
            return false;
        }
    }

    sleep(randomize(1,2));
}

bool User::status()
{
	string result, followers, following, reset_time;
	string remaining_hits, hourly_limit;

/*
 *    deque < string >
 *        tofollow(this->database->toVector( "ToFollow", "userid"));
 *    deque < string >
 *        followed(this->database->toVector( "Followed", "userid"));
 *    deque < string >
 *        unfollowed(this->database->toVector( "UnFollowed", "userid"));
 *    deque < string >
 *        myfollowers(this->database->toVector( "MyFollowers", "userid"));
 *
 */

    deque < string > tofollow(1);
    deque < string > followed(1);
    deque < string > unfollowed(1);
    deque < string > myfollowers( this->my_followers );

	cout << "\tDatabase Status :" << endl;
	cout << "\t\tFollowed     : " << followed.size() << endl;
	cout << "\t\tTo follow    : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed   : " << unfollowed.size() << endl;
	cout << "\t\tMy followers : " << myfollowers.size() << endl;

	if (this->twitterObj.accountVerifyCredGet() == true) {
		cout << "\tAccount Status     :" << endl;
		if (this->lastResponse("user.followers_count", followers) ==
		    false) {
			
			    cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << endl;
			return false;
		}
		if (this->lastResponse("user.friends_count", following) ==
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
	if (this->twitterObj.accountRateLimitGet() == true) {


		cout << "\tAPI Status : " << endl;
		if (this->lastResponse("hash.remaining-hits", remaining_hits) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.remaining-hits" <<
			    endl;
			return false;
		}

		if (this->lastResponse("hash.hourly-limit", hourly_limit) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		if (this->lastResponse("hash.reset-time", reset_time) == false) {
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
	if (this->twitterObj.accountRateLimitGet() == true) {
		if (this->lastResponse("hash.remaining-hits", temp_limit) ==
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
