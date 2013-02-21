/**
 * Common things used in various parts of the code
 * @file common.h
 */

/**
 * A structure representing the user proxy
 */
struct Proxy {
	/** ip address of proxy */
	std::string address;
	/** port number of proxy */
	std::string port;
	/** username of proxy */
	std::string username;
	/** password of proxy */
	std::string password;
};

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

/**
 * A structure to represent the user
 * object
 */
struct User {
	/** username */
	std::string username;
	/** password */
	std::string password;
	/** consumer key */
	std::string consumer_key;
	/** consumer secret */
	std::string consumer_secret;
	/** access token key */
	std::string access_token_key;
	/** access token secret */
	std::string access_token_secret;
	/** the database we will use for this
         * user
         */
	std::string db_name;
	/**
         * name of the timezone
         */
	std::string timezone;
	/**
         * sqlite database object
         */
	sqlite3pp::database db;
	/** number of followers */
	std::string followers;
	/** number of following */
	std::string following;
	/** A proxy object */
	Proxy proxy;
	/** A filter object */
	Filters filters;
	/** the twitter curl object */
	twitCurl twitterObj;
};
