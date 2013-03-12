#include "twitterfu.h"

int optionSelect()
{
	int opt = -1;

	cout << "> ";
	cin >> opt;

	if (cin.fail() == true) {
		cin.clear();
		cin.ignore(numeric_limits < streamsize >::max(),
				0x0a);
		return -1;
	}

	return opt;
}

vector < string > fileToVector(string filename)
{
	vector < string > v;
	string temp;
	fstream fs(filename.c_str(), fstream::in);

	if (fs.is_open() == false) {
		
		    cerr << "\t[-] Error : Unable to open " << filename << 
		    endl;
		return v;
	}

	while (fs.eof() == false) {
		getline(fs, temp);
		if (temp.length() != 0)
			v.push_back(temp);
	}

	fs.close();
	return v;
}

int randomize(int from, int to)
{
	return rand() % to + from;
}

bool fileExists(string filename)
{
	struct stat fi;
	return stat(filename.c_str(), &fi) == 0;
}

void optionParse(User * user, int opt)
{
	string username;
	vector < string > ids;

	switch (opt) {
	case 1:		// Get followers of a user
		{
			cout << "Username : ";
			cin >> username;

			if (removeDuplicatesInToFollow(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;

			}

			ids = user->getFollowers(username);

			if (toDB(user, ids, "ToFollow", "userid") == false) {
				cerr << "[-] Error : toDB" << endl;
				return;
			}

			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;
		}
		break;
	case 2:		// Get following of a user
		{
			cout << "Username : ";
			cin >> username;

			if (removeDuplicatesInToFollow(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = user->getFollowing(username);

			if (toDB(user, ids, "ToFollow", "userid") == false) {
				cerr << "[-] Error : toDB" << endl;
				return;
			}

			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;

			break;
		}
	case 3:		// Search
		{
			string query;
			cout << "Enter a phrase : ";
			cin.ignore();
			getline(cin, query);

			if (removeDuplicatesInToFollow(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = user->search(query);

			if (toDB(user, ids, "ToFollow", "userid") == false) {
				cerr << "[-] Error : toDB" << endl;
				return;
			}
			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;
		}
		break;
	case 4:		// follow users
		{
			if (removeDuplicatesInToFollow(user) == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			user->follow(toVector(user, "ToFollow", "userid"));
		}
		break;
	case 5:		// our status
		{
			user->status();
		}
		break;
	case 6:		// unfollow users
		{
			user->unfollow();
		}
		break;
	case 7:		// Configure
		{
			user->configure();
			
			    cout << "Rerun application to apply changes." <<
			    endl;
		}
		break;
	case 8:		// quit
		{
			cout << "\tHave a nice day!" << endl;
			exit(1);
		}
		break;
	default:
		cerr << "\t[-] Error : Invalid option" << endl;
		break;
	}
}

void optionShow()
{
	cout << "1) Get followers of a user" << endl;
	cout << "2) Get following of a user" << endl;
	cout << "3) Search by " << endl;
	cout << "4) Start following" << endl;
	cout << "5) Status" << endl;
	cout << "6) Unfollow users who haven't followed" << endl;
	cout << "7) Configure" << endl;
	cout << "8) Quit" << endl;
}

bool authenticate(User * &user)
{
	string q, authurl, pin;

	// set twitter user, pass, and consumer {key,secret}
        user->twitterObj.setTwitterUsername(user->getUsername());
	user->twitterObj.setTwitterPassword(user->getPassword());
	user->twitterObj.getOAuth().setConsumerKey(user->getConsumerKey());
	user->twitterObj.getOAuth().setConsumerSecret(user->getConsumerSecret());

	// if we already have oauth keys
	if (user->getAccessTokenKey().size() && user->getAccessTokenSecret().size()) {
		user->twitterObj.getOAuth().setOAuthTokenKey(user->getAccessTokenKey());
		user->twitterObj.getOAuth().
		    setOAuthTokenSecret(user->getAccessTokenSecret());
		return true;
	} else {		// if we don't
		// get pin
		if(user->twitterObj.oAuthRequestToken(authurl) == false) {
                        cerr << "[-] Failed while trying to get auth url" << endl;
                        return false;
                }
		cout <<
		    "Visit twitter and authorize the application then enter the PIN."
		    << endl << authurl << endl;
		cout << "PIN : ";
		cin >> pin;
		user->twitterObj.getOAuth().setOAuthPin(pin);
		user->twitterObj.oAuthAccessToken();

		// update database with access keys
		user->twitterObj.getOAuth().getOAuthTokenKey(user->getAccessTokenKey());
		user->twitterObj.getOAuth().
		    getOAuthTokenSecret(user->getAccessTokenSecret());

		q = "UPDATE Config SET access_key = \"" +
		    user->getAccessTokenKey() + "\" WHERE Id=1;";
		if (user->db.execute(q.c_str()) != 0)
			return false;
		q = "UPDATE Config SET access_secret = \"" +
		    user->getAccessTokenSecret() + "\" WHERE Id=1;";
		if (user->db.execute(q.c_str()) != 0)
			return false;
		return true;
	}

	return false;
}

int main()
{
	string error;
	vector < string > myFollowers;
	User *user = new User;
	string result, temp, query;
	int opt, remainingHits;
	struct passwd *pw = getpwuid(getuid());

	srand(time(NULL));	// random seed

	/* get users directory */
        string dbtemp = pw->pw_dir;
        dbtemp += "/.twitterfu.sql";
        user->setDBname( dbtemp );
	user->setConsumerKey("nYFCp8lj4LHqmLTnVHFc0Q");
	user->setConsumerSecret("EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4");

	/* Initalize database */
	if (initalize(user) == false) {
		cerr << "[-] Error : Unable to initalize database" << 
		    endl;
		return -1;
	}
	// we should see if the Config table has someone or not
	if (userExist(user) == false) {
		// create the username, password
		if (createUser(user) == false) {
			cerr << "Unable to create user" << endl;
			return -1;
		}
	} else {		// Get all needed values from DB
		user->setUsername(getVal(user, "Config", "username").at(0));
		user->setAccessTokenKey(getVal(user, "Config", "access_key").at(0));
		user->setAccessTokenSecret(getVal(user, "Config", "access_secret").at(0));
		user->setTimezone(getVal(user, "Config", "timezone").at(0));
		user->proxy->setAddress(getVal(user, "Config", "proxy_address").at(0));
		user->proxy->setPort(getVal(user, "Config", "proxy_port").at(0));
		user->proxy->setUsername(getVal(user, "Config", "proxy_username").at(0));
		user->proxy->setPassword(getVal(user, "Config", "proxy_password").at(0));
	}

	/* Set up proxy if found */
	if (!user->proxy->getAddress().empty() && !user->proxy->getPort().empty()) {
		user->twitterObj.setProxyServerIp(user->proxy->getAddress());
		user->twitterObj.setProxyServerPort(user->proxy->getPort());
		cout << "[+] Using proxy " << user->proxy->getAddress() << ":" << user->proxy->getPort() << endl;
		/* Set password if found */
		if (!user->proxy->getUsername().empty()
		    && !user->proxy->getPassword().empty()) {
			user->twitterObj.setProxyUserName(user->proxy->getUsername());
			user->twitterObj.setTwitterPassword(user->proxy->getPassword());
		}
	}

	/* Authenticate our user */
	if (authenticate(user) == false) {
		cerr << "[-] Failed while authenticating" << endl;
		return -1;
	}

	/* If we don't have enough hits suggest using a proxy
	 * and exit 
	 **/
	remainingHits = user->getRemainingHits();
	if (remainingHits == 0) {
		cerr <<
		    "[-] Error : You have reached the limit, maybe using a proxy might help"
		    << endl;
		if (user->configure() == false) {
			cerr << "[-] Error : Unable to configure" << 
			    endl;
			return -1;
		}
		return -1;
	}

	/* Verifying authentication */
	if (user->twitterObj.accountVerifyCredGet() == true) {
		// get following
		if (user->lastResponse("user.friends_count", user->getFollowing() ) == false) {
			    cerr <<
			    "[-] Error : Unable to find user.friends_count" <<
			    endl;
			return -1;
		}
		// get followers
		if (user->lastResponse("user.followers_count",
				 user->getFollowers()) == false) {
			cerr <<
			    "[-] Error : Unable to find user.followers_count" <<
			    endl;
			return -1;
		}
		// set timezone if not set
		if (user->getTimezone().empty()) {
			if (user->lastResponse("user.time_zone", user->getTimezone())
			    == false) {
				
				    cerr <<
				    "[-] Error : Unable to find timezone" <<
				    endl;
				return -1;
			} else {
				user->db.connect(user->getDBname().c_str());
				query =
				    "UPDATE Config SET timezone = \"" +
				    user->getTimezone() + "\";";
				if (user->db.execute(query.c_str()) != 0) {
					
					    cerr << "Unable to update timezone"
					    << endl;
				}
				user->db.disconnect();
				cout << "We have set the timezone to " <<
				    user->getTimezone() << endl;
			}
		}
	} else {
		cerr << "[-] Error : Unable to authenticate." << endl;
		if (!user->proxy->getAddress().empty() && !user->proxy->getPort().empty()) {
			cout <<
			    "If this is due to misconfiguration you can change it"
			    << endl;
			if (user->configure() == false) {
				
				    cerr << "[-] Error : Unable to configure" <<
				    endl;
				return -1;
			}
			
			    cout << "Rerun the application to apply changes." <<
			    endl;
		}

		return -1;
	}

	cout << "=====================" << endl;
	cout << "Hello @" << user->getUsername() << endl;
	cout << "Following : " << user->getFollowing() << endl;
	cout << "Followers : " << user->getFollowers() << endl;
	cout << "=====================" << endl << endl;

	/* We shall get our followers */
        myFollowers = user->getFollowers(user->getUsername());
	if (myFollowers.size() != 0) {
		cout << "Adding a result of " << myFollowers.size() <<
		    " to MyFollowers;" << endl;
		if (toDB(user, myFollowers, "MyFollowers", "userid") == false) {
			cerr << "[-] Error : Unable to toDB" << endl;
		}
	}
	// Before entering the main loop fix the databases
	if (removeDuplicatesInToFollow(user) == false) {
		cerr << "[-] Error : Unable to remove duplicates" << 
		    endl;
		return -1;
	}

	opt = 0;
	while (opt != 8) {
		optionShow();
		opt = optionSelect();
		optionParse(user, opt);
	}

	return 0;
}

void cleanLine(int n)
{
	for (int i = 0; i < n; i++)
		cout << " ";
	cout << "\xd";
	flush(cout);
}

template < class T > void
concatVectors(vector < T > &dest, vector < T > src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}
