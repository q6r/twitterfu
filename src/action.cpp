#include "action.h"

/**
 * Globals
 */
bool gotExitSignal = false;

int getRemainingHits(User * user)
{
	string temp_limit;
	int limit;

	// Get account limits remaining hits
	if (user->twitterObj.accountRateLimitGet() == true) {
		if (lastResponse(user, "hash.remaining-hits", temp_limit) ==
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

bool status(User * user)
{
	string result, followers, following, reset_time;
	string remaining_hits, hourly_limit;

	vector < string >
	    tofollow(toVector(user, "ToFollow", "userid"));
	vector < string >
	    followed(toVector(user, "Followed", "userid"));
	vector < string >
	    unfollowed(toVector(user, "UnFollowed", "userid"));
	vector < string >
	    myfollowers(toVector(user, "MyFollowers", "userid"));

	cout << "\tDatabase Status :" << endl;
	cout << "\t\tFollowed     : " << followed.size() << endl;
	cout << "\t\tTo follow    : " << tofollow.size() << endl;
	cout << "\t\tUnfollowed   : " << unfollowed.size() << endl;
	cout << "\t\tMy followers : " << myfollowers.size() << endl;

	if (user->twitterObj.accountVerifyCredGet() == true) {
		cout << "\tAccount Status     :" << endl;
		if (lastResponse(user, "user.followers_count", followers) ==
		    false) {
			
			    cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << endl;
			return false;
		}
		if (lastResponse(user, "user.friends_count", following) ==
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
	if (user->twitterObj.accountRateLimitGet() == true) {

		cout << "\tAPI Status : " << endl;
		if (lastResponse(user, "hash.remaining-hits", remaining_hits) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.remaining-hits" <<
			    endl;
			return false;
		}

		if (lastResponse(user, "hash.hourly-limit", hourly_limit) ==
		    false) {
			
			    cerr <<
			    "[-] Error : Unable to get hash.hourly-limit" <<
			    endl;
			return false;
		}

		if (lastResponse(user, "hash.reset-time", reset_time) == false) {
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

bool lastResponse(User * user, string node, string & v)
{
	string result = "";
	boost::property_tree::ptree pt;
	user->twitterObj.getLastWebResponse(result);
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

void signalHandler(int n)
{
	gotExitSignal = true;
}

vector < string > search(User * user, string query)
{
	string jsonResult;
	boost::property_tree::ptree pt;
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

vector < string > getFollowing(User * user, string username)
{
	string result, err, next_cursor;
	vector < string > ids;
	next_cursor = "-1";

	cout << "[+] Getting following of @" << username << endl;

	do {
		if (user->twitterObj.friendsIdsGet(next_cursor, username,
						   false) == true) {
			user->twitterObj.getLastWebResponse(result);
			boost::property_tree::ptree pt;
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched and error ? */
			if (lastResponse(user, "hash.error", err) == true) {
				cerr << "[-] " << err << endl;
				return ids;
			}

			/* Get next cursor */
			lastResponse(user, "id_list.next_cursor", next_cursor);

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

vector < string > getFollowers(User * user, string username)
{
	string result, err, next_cursor;
	vector < string > ids;
	boost::property_tree::ptree pt;

	next_cursor = "-1";

	cout << "[+] Getting followers of @" << username << endl;

	do {
		if (user->twitterObj.followersIdsGet(next_cursor, username,
						     false) == true) {
			user->twitterObj.getLastWebResponse(result);
			stringstream ss(result);
			read_xml(ss, pt);

			/* Catched error ? */
			if (lastResponse(user, "hash.error", err) == true) {
				cerr << "[-] " << err << endl;
				return ids;
			}

			/* Get next cursor */
			lastResponse(user, "id_list.next_cursor", next_cursor);

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

void unfollow(User * user)
{
	vector < string >
	    followers(getVal(user, "MyFollowers", "userid"));
	vector < string >::iterator it;
	vector < string > result;
	string who;
	string isfollow = "true";
	long unfollowed = 0;
	int remainingHits = 0;
	gotExitSignal = false;

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

		user->twitterObj.friendshipShow(*it, true);

		/* get followed_by and if fails check
		 * the remaining hits and reconfigure
		 **/
		if (lastResponse(user,
				 "relationship.source.followed_by",
				 isfollow) == false) {
			// check if we reached the limit
			remainingHits = getRemainingHits(user);
			if (remainingHits == 0) {
				cerr <<
				    "[-] Error : You have reached the limit how about using a proxy ?"
				    << endl;
				if (configure(user) == false) {
					
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
			if (lastResponse(user,
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
	if (toDB(user, result, "UnFollowed", "userid") == false)
		cerr << "[-] Error : Unable to write to db" << endl;
}

void follow(vector < string > to_follow, User * user)
{
	gotExitSignal = false;
	string username, error, result;
	vector < string > followed;
	vector < string >::iterator it;
	int ignored = 0;

	if (to_follow.size() == 0) {
		cerr << "(Err:Please add users to follow)" << endl;
		return;
	}
	// remove duplicates
	if (removeDuplicatesInToFollow(user) == false) {
		cerr << "(Err:Unable to remove duplicates)" << endl;
		return;
	}

	cout << to_follow.size() << " to follow" << endl;

	/* Install the signal handler */
	if (signal((int)SIGINT, signalHandler) == SIG_ERR) {
		cerr << "(Err:Unable to install signalHandler)" << 
		    endl;
		return;
	}

	for (it = to_follow.begin(); it != to_follow.end() && gotExitSignal != true; it++) {	// { Users to follow

		// follow only those that applies to the
		// by_ratio filter
		if (user->filters->mainFilter(user, *it) == true) {
			if (user->twitterObj.friendshipCreate(*it, true) == true) {	// if followed the user
				followed.push_back(*it);
				if (lastResponse(user, "user.name", username) == false) {	// if can't get username
					if (lastResponse(user, "hash.error", error) == true) {	// get hash error

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
			if (lastResponse(user, "hash.error", error) == true) {
				if (string::npos !=
				    error.find("Clients may not make")) {
					
					    cerr <<
					    "The client have reached the API limit, please try again in an hour"
					    << endl;
					break;
				}
			}
			// Who did we ignore ?
			if (lastResponse(user, "user.name", username) == true) {
				cout << "Ignored " << username;
				cleanLine(120);
			}
			ignored++;
		}
	}			// } users to follow

	/* when signal is caught or when block is over */
	gotExitSignal = false;
	if (followed.size() != 0)
		cout << endl << "We have followed " << followed.
		    size() << "/" << to_follow.size() - ignored << endl;

	if (toDB(user, followed, "Followed", "userid") == false) {
		cerr << "[-] Error : toDB" << endl;
		return;
	}
	if (ignored > 0)
		cout << "\tWe have Ignored : " << ignored << endl;
}
