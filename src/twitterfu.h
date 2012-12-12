using namespace std;

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
};

/* Globals
 */
bool gotExitSignal = false;

/* ProtoTypes
 */
vector < string > get_followers_of(twitCurl & twitterObj, string username);
vector < string > get_following_of(twitCurl & twitterObj, string username);
void concat_vectors(vector < string > &dest, vector < string > src);
bool vector_to_file(string filename, vector < string > v);
vector < string > file_to_vector(string filename);
bool config(string filename, User * user);
bool remove_duplicates(string, string, string);
bool
status(twitCurl & twitterObj, string f_tofollow, string f_followed,
       string f_unfollowed);
int option_select();
void option_parse(User * user, twitCurl & twitterObj, int opt);
void option_show();
template < class T > void
parse_lastweb_response(twitCurl & twitterObj, string node, T & v);
void unfollow(twitCurl & twitterObj, string filename, string username);
void follow(twitCurl & twitterObj, vector < string > to_follow);
void signalhandler(int n);
bool create_cache(string f_tofollow, string f_followed, string f_unfollowed);

