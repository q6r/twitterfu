#include "twitterfu.h"

using namespace std;

/* @method      : filter::by_ratio()
 * @description : find user followers, following ratio
 * and predict if he have a higher probability of follow-
 * ing back
 * @input       : userid
 * @output      : 
 *      True if :
 *              1) following more than followers
 *              else 2) following:followers ratio is >= 65%
 *      False if otherwise;
 */
bool filter::by_ratio(User * user, string userid)
{

	string replyMsg;
	string temp_following, temp_followers;
	long double following, followers;

	if (user->twitterObj.userGet(userid, true) == false)
		return false;
	user->twitterObj.getLastWebResponse(replyMsg);

	if (parse_lastweb_response(user, "user.followers_count", temp_followers)
	    == false)
		return false;
	if (parse_lastweb_response(user, "user.friends_count", temp_following)
	    == false)
		return false;

	stringstream sa(temp_following);
	stringstream sb(temp_followers);
	sa >> following;
	sb >> followers;

	if (following >= followers) {
		return true;
	} else {
		long double result = (following / followers) * 100;
		if (result >= 50)
			return true;
		else
			return false;

	}

	return false;
}
