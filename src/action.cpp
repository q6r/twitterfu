#include "twitterfu.h"

/* Globals
 */
bool gotExitSignal = false;


/* @method      : action::status
 * @description : Show database, account and API status.
 * @input       : user
 * @output      : true if successful otherwise false
 */
bool action::status(User * user)
{
	std::string result, followers, following, reset_time;
	std::string remaining_hits, hourly_limit;

	std::vector < std::string > tofollow(database::toVector(user, "ToFollow", "userid"));
	std::vector < std::string > followed(database::toVector(user, "Followed", "userid"));
	std::vector < std::string > unfollowed(database::toVector(user, "UnFollowed", "userid"));
        std::vector < std::string > myfollowers(database::toVector(user, "MyFollowers", "userid"));

	std::cout << "\tDatabase Status :" << std::endl;
	std::cout << "\t\tFollowed     : " << followed.size() << std::endl;
	std::cout << "\t\tTo follow    : " << tofollow.size() << std::endl;
	std::cout << "\t\tUnfollowed   : " << unfollowed.size() << std::endl;
        std::cout << "\t\tMy followers : " << myfollowers.size() << std::endl;

	if (user->twitterObj.accountVerifyCredGet() == true) {
		std::cout << "\tAccount Status     :" << std::endl;
		if (lastResponse(user, "user.followers_count",
				      followers) == false) {
			std::cerr <<
			    "\t[-] Error : Unable to find the followers_count"
			    << std::endl;
			return false;
		}
		if (lastResponse(user, "user.friends_count", following) ==
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
		if (lastResponse
		    (user, "hash.remaining-hits", remaining_hits) == false) {
			std::cerr << "[-] Error : Unable to get hash.remaining-hits"
			    << std::endl;
			return false;
		}

		if (lastResponse
		    (user, "hash.hourly-limit", hourly_limit) == false) {
			std::cerr << "[-] Error : Unable to get hash.hourly-limit" <<
			    std::endl;
			return false;
		}

		if (lastResponse(user, "hash.reset-time", reset_time) ==
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


/*
 * @method           : action::lastResponse
 * @description      : parse the last web response and get a
 * value of a node, save the value in T v
 * @input            : user, node, v
 * @output           : true if got the result false otherwise
 */
bool action::lastResponse(User * user, std::string node, std::string & v)
{
	std::string result = "";
	boost::property_tree::ptree pt;
	user->twitterObj.getLastWebResponse(result);
	std::stringstream ss(result);

	try {
		read_xml(ss, pt);
		v = pt.get < std::string > (node.c_str());
	}
	catch(std::exception const &e) {
		return false;
	}

	return true;
}

/*
 * @method      : action::signalHandler
 * @description : Handler for follow/unfollow to set a exitFlag
 * @input       : n the catched signal number
 * @outpu       : None
 */
void action::signalHandler(int n)
{
	gotExitSignal = true;
}

/* @method      : action::search
 * @description : Will take a query and return a std::vector of
 * user ids to be added to the to follow list. This function
 * is limited to 15 results since pages are not supported
 * by twitCurl.
 * @input       : User, query
 * @output      : std::vector of user ids
 */
std::vector < std::string > action::search(User * user, std::string query)
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
 * @method      : action::getFollowing
 * @description : Fetch the following of a user
 * and create a std::vector of their userids and return that std::vector
 * @input       : user, username
 * @output      : std::vector of userIDs of the username's following.
 */
std::vector < std::string > action::getFollowing(User * user, std::string username)
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
			if (lastResponse(user, "hash.error", err) == true) {
				std::cerr << "[-] " << err << std::endl;
				return ids;
			}

			/* Get next cursor */
			lastResponse(user, "id_list.next_cursor",
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
 * @method      : action::getFollowersOf
 * @description : Get the followers of a user
 * and create a std::vector of their userIDs and return that std::vector
 * @input       : user, username
 * @output      : std::vector of userIDs of the username's followers
 */
std::vector < std::string > action::getFollowers(User * user, std::string username)
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
			if (lastResponse(user, "hash.error", err) == true) {
				std::cerr << "[-] " << err << std::endl;
				return ids;
			}

			/* Get next cursor */
			lastResponse(user, "id_list.next_cursor",
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

/*
 * @method      : action::unfollow
 * @description : Unfollow users who haven't followed me back
 * @input       : user 
 * @output      : None
 */
void action::unfollow(User * user)
{
	std::vector < std::string > followers(getFollowing(user, user->username));
	std::vector < std::string >::iterator it;
	std::vector < std::string > result;
	std::string who;
	std::string isfollow = "true";
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

		if (lastResponse(user,
				      "relationship.source.followed_by",
				      isfollow) == false) {
			std::cerr <<
			    "(Err: Unable to find relationship.source.followed_by)"
			    << std::endl;
			break;
		}

		if (isfollow == "false") {
			if (lastResponse(user,
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
		isfollow = "true";
	}

	// restart the exit signal flag
	gotExitSignal = false;
	std::cout << "We have unfollowed " << unfollowed << "/" << followers.size()
	    << std::endl;

	// write results to db
	if (database::toDB(user, result, "UnFollowed", "userid") == false)
		std::cerr << "[-] Error : Unable to write to db" << std::endl;
}

/* @method      : action::follow
 * @description : It will follow a std::vector of user ids
 * @input       : user ids std::string std::vector, and user
 * @output      : None
 */
void action::follow(std::vector < std::string > to_follow, User * user)
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
	if (database::removeDuplicatesInToFollow(user) == false) {
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
				if (lastResponse(user, "user.name", username) == false) {	// if can't get username
					if (lastResponse(user, "hash.error", error) == true) {	// get hash error

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
			if (lastResponse(user, "hash.error", error) ==
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
			if (lastResponse(user, "user.name", username) ==
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

	if (database::toDB(user, followed, "Followed", "userid") == false) {
		std::cerr << "[-] Error : database::toDB" << std::endl;
		return;
	}
	if (ignored > 0)
		std::cout << "\tWe have Ignored : " << ignored << std::endl;
}


