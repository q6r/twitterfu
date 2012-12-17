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
	long
	 followers;
	long
	 following;
	Cache cache;
	Proxy proxy;
	twitCurl twitterObj;
};

/* ProtoTypes
 */
vector < string > get_followers_of(User * user, string username);
vector < string > get_following_of(User * user, string username);
template < class T > void concat_vectors(vector < T > &dest, vector < T > src);
template < class T > bool vector_to_file(string filename, vector < T > v);
vector < string > file_to_vector(string filename);
bool config(string filename, User * user);
bool remove_duplicates(User * user);
bool status(User * user);
int option_select();
void option_parse(User * user, int opt);
void option_show();
template < class T >
    bool parse_lastweb_response(User * user, string node, T & v);
void unfollow(User * user);
void follow(vector < string > to_follow, User * user);
void signalhandler(int n);
bool create_cache(User * user);
bool file_exists(string filename);
int randomize(int from, int to);
bool authenticate(User * user);

/* filters */
namespace filter {
	bool by_ratio(User * user, string userid);
};
