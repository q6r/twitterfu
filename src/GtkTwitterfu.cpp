#include "GtkTwitterfu.h"

GtkTwitterfu::GtkTwitterfu() :
    vbox(Gtk::ORIENTATION_VERTICAL),
    button_find_followers("Find followers"),
    button_find_following("Find following"),
    button_start_following("Start following"),
    button_stop_following("Stop following"),
    button_quit("Quit"),
    label_status("Status:"),
    input(NULL),
    follow_worker(NULL)
{

    string *username = new string; // Will be passed to GtkLogin 
    string *password = new string; // Will be passed to GtkLogin

    // Create Login window
    // TODO this is very very bad, if kit is deleted
    // segfault! find another way
    Gtk::Main *kit    = new Gtk::Main(true);
    GtkLogin *loginUI = new GtkLogin(username, password);
    kit->run(*loginUI);
    delete loginUI;
    /*
     *delete kit;
     */

    this->set_title("Twitterfu");
    this->set_border_width(10);
    this->set_default_size(400,200);

    this->add(vbox);

    // scrolled if necessary for treeview
    scrolledwindow.add(treeview);
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // Ref to treemodel
    refTreeModel              = Gtk::ListStore::create(columns);
    treeview.set_model(refTreeModel);

    // align label
    label_status.set_alignment(0, 0 );

    // add to vbox
    vbox.pack_start(scrolledwindow);
    vbox.pack_start(buttonbox, Gtk::PACK_SHRINK);
    vbox.pack_start(label_status, Gtk::PACK_SHRINK);

    // add button to buttonbox
    buttonbox.pack_start(button_find_followers, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_find_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_start_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_stop_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_quit, Gtk::PACK_SHRINK);

    buttonbox.set_layout(Gtk::BUTTONBOX_END);

    // add the treeview's view columns:
    // this will be shown
    treeview.append_column("ID", columns.m_col_id);

    // create button singals()
    button_find_followers.signal_clicked().connect(
            sigc::mem_fun(*this,
                &GtkTwitterfu::on_button_find_followers));
    button_find_following.signal_clicked().connect(
            sigc::mem_fun(*this,
                &GtkTwitterfu::on_button_find_following));
    button_start_following.signal_clicked().connect(
            sigc::mem_fun(*this, &GtkTwitterfu::on_button_start_following));

    button_stop_following.signal_clicked().connect(
            sigc::mem_fun(*this, &GtkTwitterfu::on_button_stop_following));
    button_quit.signal_clicked().connect( sigc::mem_fun(*this,
                &GtkTwitterfu::on_button_quit));

    //////// Getting users
    //
    user = new User(*username, *password, "nYFCp8lj4LHqmLTnVHFc0Q", "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4");

    // TODO check auth ..etc
    if(user->isAuthenticated() == false) {
        std::cout << "Error : Unable to authenticate" << std::endl;
    }

    // Verify and if failed keep asking for login until authenticated
    // and verified.
    if(user->verify() == false) {
        std::cout << "Unable to verify" << std::endl;
        // Then relogin authenticate!
        do {
            delete user;
            loginUI = new GtkLogin(username, password, "Invalid username or password");
            kit->run(*loginUI);
            delete loginUI;
            user = new User(*username, *password, "nYFCp8lj4LHqmLTnVHFc0Q", "EbTvHApayhq9FRPHzKU3EPxyqKgGrNEwFNssRo5UY4");

        } while(!user->verify() || !user->isAuthenticated() );
    }


    //////// Getting users

    this->show_all_children();
}

void GtkTwitterfu::setStatus(Gtk::Label &label, Glib::ustring text) {
    Glib::ustring _text = "Status : " + text;
    label.set_text( _text );
}

GtkTwitterfu::ModelColumns::ModelColumns() {
    this->add(m_col_id);
}

void GtkTwitterfu::addID(Glib::ustring id) {
    Gtk::TreeModel::Row row   = *(refTreeModel->append());
    row[columns.m_col_id] = id;
}

void GtkTwitterfu::find_followers() {
    char *username = input->getResult();
    if(username[0] == 0x00) {
        delete username; username = NULL;
        delete input; input = NULL;
        return;
    }


    // Status
    this->setStatus( this->label_status, "Getting followers of " + Glib::ustring(username));

    deque< string > user_followers = user->getFollowing(username);
    for_each(user_followers.begin(), user_followers.end(), [&](string id_) {
            this->addID(id_);
            });

    if(user_followers.size() == 0) {
        this->setStatus( this->label_status, "Didn't get any users");
    } else {
        std::stringstream str;
        str << "Added " << user_followers.size() << " ids to follow";
        this->setStatus( this->label_status, str.str());
    }

    // Append this to the users to follow to the end
    users_to_follow.insert(users_to_follow.end(), user_followers.begin(), user_followers.end());
    std::cout << "Total users to follow " << users_to_follow.size() << std::endl;

    delete username;
    username = NULL;
    delete input;
    input = NULL;
}

void GtkTwitterfu::find_following() {
    char *username = input->getResult();
    if(username[0] == 0x00) {
        delete username; username = NULL;
        delete input; input = NULL;
        return;
    }

    // Status
    this->setStatus( this->label_status, "Getting following of " + Glib::ustring(username));

    deque < string > user_following = user->getFollowers(username);
    for_each(user_following.begin(), user_following.end(), [&](string id_) {
            this->addID(id_);
            });

    if(user_following.size() == 0) {
        this->setStatus( this->label_status, "Didn't get any users");
    } else
    {
        std::stringstream str;
        str << "Added " << user_following.size() << " ids to follow";
        this->setStatus( this->label_status, str.str() );
    }

    // Append this to the users to follow
    users_to_follow.insert(users_to_follow.end(), user_following.begin(), user_following.end());
    std::cout << "Total users to follow " << users_to_follow.size() << std::endl;

    delete username;
    username = NULL;
    delete input;
    input = NULL;
}

void GtkTwitterfu::on_button_find_followers() {
    
    // InputWorker still waiting for input.
    if(input != NULL) {
        std::cout << "waiting for input" << std::endl;
        return;
    }

    input = new InputWorker("Enter username");
    input->sig_done.connect( sigc::mem_fun(*this, &GtkTwitterfu::find_followers));
    input->start();
}

void GtkTwitterfu::on_button_find_following() {
    // Input worker still waiting for input.
    if(input != NULL) {
        std::cout << "waiting for input" << std::endl;
        return;
    }

    input = new InputWorker("Enter username");
    input->sig_done.connect( sigc::mem_fun(*this, &GtkTwitterfu::find_following));
    input->start();
}

void GtkTwitterfu::followed_user() {
    std::cout << "Worker done!" << std::endl;
    delete follow_worker;
    follow_worker = NULL;
}

void GtkTwitterfu::on_button_start_following() {
    // We should have a worker to do the following
    // in a thread mode.. let's create a class for that.
    
    
    // no one to follow
    if(users_to_follow.size() == 0) {
        this->setStatus(this->label_status, "There's no one to follow!");
        return;
    }

    if(follow_worker != NULL) {
        std::cout << "Please wait follower is working" << std::endl;
        return;
    }

    follow_worker = new FollowWorker(this->user, this->users_to_follow);
    follow_worker->sig_done.connect( sigc::mem_fun(*this, &GtkTwitterfu::followed_user));
    follow_worker->start();
}

void GtkTwitterfu::removeID(Glib::ustring id) {
    Gtk::TreeModel::Children rows = refTreeModel->children();
    Gtk::TreeModel::iterator it;

    // Search in the treemodel for id if found erase
    std::for_each(rows.begin(), rows.end(), [&](Gtk::TreeModel::iterator it) {
            Gtk::TreeModel::Row row = *it;
            const char *current_id = Glib::ustring(row[columns.m_col_id]).c_str();

            if(strcmp(current_id, id.c_str()) == 0) {
                refTreeModel->erase(it);
            }
            
            });
}

// Stop the following worker by setting it's flag,
// then deallocate it so we can use it sometime later
void GtkTwitterfu::on_button_stop_following() {
    if(follow_worker != NULL) {
        follow_worker->stopThread();
        delete follow_worker;
        follow_worker = NULL;
    }
}

void GtkTwitterfu::on_button_quit() {
    this->hide();
}

GtkTwitterfu::~GtkTwitterfu() {
}
