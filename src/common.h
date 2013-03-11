#ifndef COMMON_H_
#define COMMON_H_
#include <iostream>
#include "sqlite3pp.h"
#include <twitcurl.h>

using namespace std;

/**
 * Common things used in various parts of the code
 * @file common.h
 */

/**
 * A structure to represent a user filter
 */
struct Filters {
	/** have profile picture */
	bool profilePicture;
	/** have description */
	bool description;
	/** is the profile protected */
	bool protectedProfile;
	/** falls in our follow ratio */
	bool followRatio;
	/** is near us */
	bool nearTimezone;
};
#endif
