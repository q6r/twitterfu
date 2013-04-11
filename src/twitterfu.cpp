#include "twitterfu.h"

int main(int argc, char *argv[]) {

    Glib::thread_init();
    Glib::RefPtr< Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.twitterfu");

    /*
     *GtkTwitterfu ui;
     */
    GtkTwitterfu ui;
    return app->run(ui);

    
/*
 *    deque < string > myFollowers;
 *    int opt;
 *    struct passwd *pw = getpwuid(getuid());
 *    string dbtemp = pw->pw_dir;
 *    dbtemp += "/.twitterfu.sql";
 *    User *user;
 *    srand(time(NULL));	// random seed
 *        
 *    // Create a user
 *    user = new User(dbtemp, "nYFCp8lj4LHqmLTnVHFc0Q", "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4");
 *
 *    // Check if user authenticated
 *    if(user->isAuthenticated() == false) {
 *        cerr << "[-] Failed to authenticate user" << endl;
 *        return -1;
 *    }
 *
 *    [> If we don't have enough hits suggest using a proxy and exit <]
 *    if(user->reachedLimit()) {
 *        cerr << "[-] Reached limit" << endl;
 *        // reconfigure
 *        if(user->configure() == false) {
 *            cerr << "[-] Unable to configure" << endl;
 *            return -1;
 *        }
 *        return -1;
 *    }
 *
 *    [> Verifying authentication <]
 *    if(!user->verify()) {
 *        cerr << "[-] Error : Unable to authenticate/verify." << endl;
 *        // if proxy is set might need to change it because it's
 *        // causing the problem
 *        if (!user->proxy->get("address").empty() && !user->proxy->get("port").empty()) {
 *            cout << "If this is due to misconfiguration you can change it" << endl;
 *            // configure the user
 *            if (user->configure() == false) {
 *                cerr << "[-] Error : Unable to configure" << endl;
 *                return -1;
 *            }
 *            cout << "Rerun the application to apply changes." <<endl;
 *        }
 *        return -1;
 *    }
 *
 *    cout << "=====================" << endl;
 *    cout << "Hello @" << user->get("username") << endl;
 *    cout << "Following : " << user->get("following") << endl;
 *    cout << "Followers : " << user->get("followers") << endl;
 *    cout << "=====================" << endl << endl;
 *
 *    [> We shall get our followers <]
 *    myFollowers = user->getFollowers(user->get("username"));
 *    if (myFollowers.size() != 0) {
 *        cout << "Adding a result of " << myFollowers.size() << " to MyFollowers;" << endl;
 *        if (user->database->toDB( myFollowers, "MyFollowers", "userid") == false) {
 *            cerr << "[-] Error : Unable to toDB" << endl;
 *        }
 *    }
 *    // Before entering the main loop fix the databases
 *    if (user->database->removeDuplicatesInToFollow() == false) {
 *        cerr << "[-] Error : Unable to remove duplicates" << endl;
 *        return -1;
 *    }
 *
 *    opt = 0;
 *    while (opt != 8) {
 *        optionShow();
 *        opt = optionSelect();
 *        optionParse(user, opt);
 *    }
 */

    /*
	 *return 0;
     */
}
