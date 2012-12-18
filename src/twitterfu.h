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

using namespace std;

struct Cache {
	string to_follow;
	string followed;
	string unfollowed;
	string oauth_key;
	string oauth_secret;
};

struct Proxy {
	string address;
	string port;
	string username;
	string password;
};

struct User {
	string username;
	string password;
	string consumer_key;
	string consumer_secret;
	string access_token_key;
	string access_token_secret;
	long followers;
	long following;
	Cache cache;
	Proxy proxy;
	twitCurl twitterObj;
};

/* ProtoTypes
 */
vector < string > getFollowersOf(User * user, string username);
vector < string > getFollowingOf(User * user, string username);
template < class T > void concatVectors(vector < T > &dest, vector < T > src);
template < class T > bool vectorToFile(string filename, vector < T > v);
vector < string > fileToVector(string filename);
bool config(string filename, User * user);
bool removeDuplicates(User * user);
bool status(User * user);
int optionSelect();
void optionParse(User * user, int opt);
void optionShow();
template < class T > bool parseLastResponse(User * user, string node, T & v);
void unfollow(User * user);
void follow(vector < string > to_follow, User * user);
void signalHandler(int n);
bool createCache(User * user);
bool fileExists(string filename);
int randomize(int from, int to);
bool authenticate(User * user);
void cleanLine(int n);

/* filters */
namespace filter {
	bool by_ratio(User * user, string userid);
};
