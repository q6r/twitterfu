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

// TODO
// add label for status,
// add button to view current status -> (Create new window for that)
// ...MMm
//
class GtkTwitterfu : public Gtk::Window {
    public:
        GtkTwitterfu();
        virtual ~GtkTwitterfu();
        void addID(Glib::ustring id); /* Add id to treeview */ 
        void removeID(Glib::ustring id); /* Remove id from treeview */ 
    private:
        void on_button_quit();
        void on_button_find_followers();
        void on_button_find_following();
        void on_button_start_following();
        void on_button_stop_following();
        void get_target();
        void find_followers();
        void find_following();
        void setStatus(Gtk::Label &label, Glib::ustring text);
        /*
         *void start_following();
         *void stop_following();
         */
        Gtk::Button button_find_followers;
        Gtk::Button button_find_following;
        Gtk::Button button_start_following;
        Gtk::Button button_stop_following;
        Gtk::Button button_quit;
        Gtk::Label label_status;

        // treeview model
        class ModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                ModelColumns();
                Gtk::TreeModelColumn<Glib::ustring> m_col_id;
        };
        ModelColumns columns; // end of tv model

        //child widgets
        Gtk::Box vbox;
        Gtk::TreeView treeview;
        Gtk::ScrolledWindow scrolledwindow;

        Glib::RefPtr< Gtk::ListStore> refTreeModel;
        Gtk::ButtonBox buttonbox;

        InputWorker *input;


        // twitterfu
        deque< string > myFollowers;
        struct passwd *pw;
        string dbtemp;
        User *user;
        deque< string > users_to_follow;   /* The users to follow */ 
};
#endif
