/* Twitterfu
 *
 * This file is part of Twitterfu.
 *
 * Twitterfu is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Twitterfu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Twitterfu. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <curl/curl.h>
#include <twitcurl.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sys/stat.h>
#include <ctime>
#include <algorithm>
#include <vector>
#include "sqlite3pp.h"

struct Proxy {
	std::string address;
	std::string port;
	std::string username;
	std::string password;
};

struct Filters {
	bool profilePicture;
	bool description;
	bool protectedProfile;
	bool followRatio;
	bool nearTimezone;
};

struct User {
	std::string username;
	std::string password;
	std::string consumer_key;
	std::string consumer_secret;
	std::string access_token_key;
	std::string access_token_secret;
	std::string db_name;
	std::string timezone;
	sqlite3pp::database db;	// sqlite3pp db
	std::string followers, following;
	Proxy proxy;
	Filters filters;
	twitCurl twitterObj;
};

/* action */
namespace action {
	int getRemainingHits(User * user);
	bool lastResponse(User * user, std::string node, std::string & v);
	void signalHandler(int n);
	void follow(std::vector < std::string > to_follow, User * user);
	void unfollow(User * user);
	 std::vector < std::string > getFollowers(User * user,
						  std::string username);
	 std::vector < std::string > getFollowing(User * user,
						  std::string username);
	 std::vector < std::string > search(User * user, std::string what);
	bool status(User * user);
}
/* database */ namespace database {
	bool removeDuplicatesInToFollow(User * user);
	bool purgeTable(User * user, std::string table);
	 std::vector < std::string > toVector(User * user, std::string table,
					      std::string value);
	bool createUser(User * user);
	 std::vector < std::string > getVal(User * user, std::string table,
					    std::string col);
	bool initalize(User * user);
	bool toDB(User * user, std::vector < std::string > v, std::string table,
		  std::string values);
	bool userExist(User * user);
}
/* filters */ namespace filter {
	bool main(User * user, std::string userid);
	bool predict_timezone(User * user, std::string timezones);
	void filter_list(User * user);
}
/* ProtoTypes
 */ bool change_proxy(User * user, std::string address, std::string port,
		      std::string username, std::string password);
bool configure(User * user);
template < class T > void concatVectors(std::vector < T > &dest,
					std::vector < T > src);
std::vector < std::string > fileToVector(std::string filename);
int optionSelect();
void optionParse(User * user, int opt);
void optionShow();
bool fileExists(std::string filename);
int randomize(int from, int to);
bool authenticate(User * user);
void cleanLine(int n);
