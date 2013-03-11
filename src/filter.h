#ifndef FILTER_H_
#define FILTER_H_
#include <string>
#include "twitterfu.h"
#include "action.h"
#include "common.h"
#include "User.h"


using namespace std;
class User;

/**
 * Filters to filter in process of following
 * @file filter.h
 */

/**
 * We filter users depending on 5 rules
 * 1) have a profile picture +1
 * 2) have description +1
 * 3) not protected profile +1
 * 4) following>follower or following:followers ratio 75% +1
 * 5) Near our timezone by -4 or +4 timezones +1
 * @param user the user object
 * @param userid the user id to apply filters to it
 * @return true if successfuly filtered
 */
bool mainfilter(User * user, string userid);
/**
 * Check if user's timezone is near us by -4 or +4 timezones
 * the timezones in the vector is pushed in order of their
 * distance.
 *
 * @param user the user object
 * @param timezones of the userid
 * @return true if able to predict time zone
 */
bool predict_timezone(User * user, string timezones);
/**
 * Show a list of filters and help
 * in toggling them.
 * @param user the user object
 */
void filter_list(User * user);
#endif
