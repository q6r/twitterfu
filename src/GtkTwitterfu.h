#ifndef GTK_TWITTERFU_
#define GTK_TWITTERFU_

#include "gtkmm.h"
#include "User.h"
#include <pwd.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <deque>
#include <algorithm>

class GtkTwitterfuInput : public Gtk::Window {
    public:
        GtkTwitterfuInput(Glib::ustring text_info);
        ~GtkTwitterfuInput();
        char *getResult();
    private:
        void on_button_okay();
        char *result;
        // child widgets
        Gtk::Box vbox;
        Gtk::Button button_okay;
        Gtk::Entry entry;
    protected:
        virtual bool on_delete_event( GdkEventAny *event ) {
            delete this;
            return false;
        }
};

class InputWorker : public GtkTwitterfuInput {
    public:
        InputWorker(Glib::ustring username);
        void start();
        ~InputWorker();
        Glib::Dispatcher sig_done;
    protected:
        void run();
        Glib::Thread * thread;
        Glib::Mutex mutex;
        bool stop;
};

// Follow user worker this will have access to 
// GtkTwitterfu so it can add/remove/change shit to the ui
class FollowWorker {
    public:
        // TODO follow worker should also have access to the tree
        // model in GtkTwitterfu so it can update it of followed users
        // it will user GtkTwitterfu::removeID for each followed id
        FollowWorker(User *_user, deque<string> _id);
        /**
         * Start thread
         */
        void start();
        /**
         * Stop thread
         */
        void stopThread();
        ~FollowWorker();
        // Dispatcher when done, TODO Do we need one ?
        Glib::Dispatcher sig_done;
    private:
        void run();
        Glib::Thread * thread;
        Glib::Mutex mutex;
        bool stop;
        deque<string> ids;   /* IDs to follow */ 
        User *user;         /* This is a ptr to the object in gtktwitterfu */
};

// TODO
// add button to view current status -> (Create new window for that)
// ...MMm
//
class GtkTwitterfu : public Gtk::Window {
    public:
        GtkTwitterfu();
        virtual ~GtkTwitterfu();
        void addID(Glib::ustring id);       /* Add id to treeview */ 
        void removeID(Glib::ustring id);    /* Remove id from treeview */ 
    private:
        void on_button_quit();              /* Just quit */ 
        void on_button_find_followers();    /* InputWorker get input */ 
        void on_button_find_following();    /* InputWorker get input */ 
        void on_button_start_following();   /* Start the FollowingWorker */ 
        void on_button_stop_following();    /* Stop the FollowingWorker thread if running */ 
        void find_followers();              /* Works after InputWorker sigdone to get followers */ 
        void find_following();              /* Works after InputWorker sigdone to get following */ 
        void setStatus(Gtk::Label &label, Glib::ustring text);
        void followed_user(); // Called when followWorker is done.

        /* Buttons and labels */ 
        Gtk::Button button_find_followers;
        Gtk::Button button_find_following;
        Gtk::Button button_start_following;
        Gtk::Button button_stop_following;
        Gtk::Button button_quit;
        Gtk::Label label_status;

        /*
         * treeview model
         */
        class ModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                ModelColumns();
                Gtk::TreeModelColumn<Glib::ustring> m_col_id;
        };
        ModelColumns columns; // end of tv model

        /*
         * Some widgets
         */
        Gtk::Box vbox;
        Gtk::TreeView treeview;
        Gtk::ScrolledWindow scrolledwindow;

        Glib::RefPtr< Gtk::ListStore> refTreeModel;
        Gtk::ButtonBox buttonbox;

        InputWorker *input;           /* The inputWorker to get work in another thread and not block : separate window */ 
        FollowWorker *follow_worker;  /* The FollowWorker to follow and report ..etc */ 

        // twitterfu
        deque< string > myFollowers;
        struct passwd *pw;
        string dbtemp;
        User *user;
        deque< string > users_to_follow;   /* The users to follow */ 
};
#endif
