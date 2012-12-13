using namespace std;

struct Cache {
	string to_follow;
	string followed;
	string unfollowed;
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
	twitCurl twitterObj;
};

/* Globals
 */
bool gotExitSignal = false;

/* ProtoTypes
 */
vector < string > get_followers_of(User * user, string username);
vector < string > get_following_of(User * user, string username);
void concat_vectors(vector < string > &dest, vector < string > src);
bool vector_to_file(string filename, vector < string > v);
vector < string > file_to_vector(string filename);
bool config(string filename, User * user);
bool remove_duplicates(User * user);
bool status(User * user);
int option_select();
void option_parse(User * user, int opt);
void option_show();
template < class T > bool
parse_lastweb_response(User * user, string node, T & v);
void unfollow(User * user);
void follow(vector < string > to_follow, User * user);
void signalhandler(int n);
bool create_cache(User * user);
bool file_exists(string filename);
int randomize(int from, int to);
