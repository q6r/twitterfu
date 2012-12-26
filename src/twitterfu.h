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
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sys/stat.h>
#include <ctime>
#include "sqlite3pp.h"

using namespace std;
using namespace sqlite3pp;

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
	string db_name;
	database db;		// sqlite3pp db
	long followers;
	long following;
	Proxy proxy;
	twitCurl twitterObj;
};

/* ProtoTypes
 */
vector < string > getValFromDB(User * user, string table, string col);
bool initalizeDatabase(User * user);
bool vectorToDB(User * user, vector < string > v, string table, string values);
vector < string > dbToVector(User * user, string table, string value);
vector < string > getFollowersOf(User * user, string username);
vector < string > getFollowingOf(User * user, string username);
template < class T > void concatVectors(vector < T > &dest, vector < T > src);
vector < string > fileToVector(string filename);
bool removeDuplicates(User * user);
bool status(User * user);
int optionSelect();
void optionParse(User * user, int opt);
void optionShow();
bool userExistInDB(User * user);
bool createUser(User * user);
template < class T > bool parseLastResponse(User * user, string node, T & v);
void unfollow(User * user);
void follow(vector < string > to_follow, User * user);
void signalHandler(int n);
bool fileExists(string filename);
int randomize(int from, int to);
bool authenticate(User * user);
void cleanLine(int n);
vector < string > search(User * user, string what);

/* filters */
namespace filter {
	bool by_ratio(User * user, string userid);
};
