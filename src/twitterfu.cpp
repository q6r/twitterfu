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

void optionParse(User * user, int opt)
{
	string username;
	deque < string > ids;

	switch (opt) {
	case 1:		// Get followers of a user
		{
			cout << "Username : ";
			cin >> username;

			if (user->database->removeDuplicatesInToFollow() == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;

			}

			ids = user->getFollowers(username);

			if (user->database->toDB(ids, "ToFollow", "userid") == false) {
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

			if (user->database->removeDuplicatesInToFollow() == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = user->getFollowing(username);

			if (user->database->toDB( ids, "ToFollow", "userid") == false) {
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

			if (user->database->removeDuplicatesInToFollow() == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			ids = user->search(query);

			if (user->database->toDB( ids, "ToFollow", "userid") == false) {
				cerr << "[-] Error : toDB" << endl;
				return;
			}
			cout << "We have added " << ids.size() <<
			    " new to follow" << endl;
		}
		break;
	case 4:		// follow users
		{
			if (user->database->removeDuplicatesInToFollow() == false) {
				cerr <<
				    "[-] Error : Unable to remove duplicates" <<
				    endl;
				return;
			}

			user->follow(user->database->toVector("ToFollow", "userid"));
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

int main()
{
	string error;
	deque < string > myFollowers;
	string result, temp, query;
	int opt;
	struct passwd *pw = getpwuid(getuid());
    string dbtemp = pw->pw_dir;
    dbtemp += "/.twitterfu.sql";
	User *user;
	srand(time(NULL));	// random seed
        
    // Create a user
    user = new User(dbtemp, "nYFCp8lj4LHqmLTnVHFc0Q", "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4");

    // Check if user authenticated
    if(user->isAuthenticated() == false) {
        cerr << "[-] Failed to authenticate user" << endl;
        return -1;
    }

	/* If we don't have enough hits suggest using a proxy and exit */
    if(user->reachedLimit()) {
        cerr << "[-] Reached limit" << endl;
        // reconfigure
        if(user->configure() == false) {
            cerr << "[-] Unable to configure" << endl;
            return -1;
        }
        return -1;
    }

	/* Verifying authentication */
    if(user->verify() == true) {
        // get and set following
        if (user->lastResponse("user.friends_count", temp) == false) {
            cerr << "[-] Error : Unable to find user.friends_count" << endl;
            return -1;
        } else {
            user->set("following", temp );
        }
        // get and set followers
        if (user->lastResponse("user.followers_count",temp) == false) {
            cerr << "[-] Error : Unable to find user.followers_count" << endl;
            return -1;
        } else {
            user->set("followers", temp );
        }
        // get and set timezone if not set
        if (user->get("timezone").empty()) {
            if (user->lastResponse("user.time_zone", temp) == false) {
                cerr << "[-] Error : Unable to find timezone" << endl;
                return -1;
            } else {
                // if there's timezone put it in db
                user->set("timezone", temp );
                user->database->setupTimezone( user->get("timezone") );
            }
        }
	} else { // Unable to verify/authenticate
		cerr << "[-] Error : Unable to authenticate." << endl;
		if (!user->proxy->get("address").empty() && !user->proxy->get("port").empty()) {
			cout << "If this is due to misconfiguration you can change it" << endl;
            // configure the user
			if (user->configure() == false) {
                cerr << "[-] Error : Unable to configure" << endl;
				return -1;
			}
			
            cout << "Rerun the application to apply changes." <<endl;
		}
		return -1;
	}

	cout << "=====================" << endl;
	cout << "Hello @" << user->get("username") << endl;
	cout << "Following : " << user->get("following") << endl;
	cout << "Followers : " << user->get("followers") << endl;
	cout << "=====================" << endl << endl;

	/* We shall get our followers */
    myFollowers = user->getFollowers(user->get("username"));
	if (myFollowers.size() != 0) {
		cout << "Adding a result of " << myFollowers.size() << " to MyFollowers;" << endl;
		if (user->database->toDB( myFollowers, "MyFollowers", "userid") == false) {
			cerr << "[-] Error : Unable to toDB" << endl;
		}
	}
	// Before entering the main loop fix the databases
	if (user->database->removeDuplicatesInToFollow() == false) {
		cerr << "[-] Error : Unable to remove duplicates" << endl;
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
