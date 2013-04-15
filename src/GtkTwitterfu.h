#ifndef GTK_TWITTERFU_
#define GTK_TWITTERFU_

#include "gtkmm.h"
#include "User.h"
#include <iostream>
#include <cstring>
#include <ctime>
#include <deque>
#include <algorithm>
#include "GtkLogin.h"
#include "InputWorker.h"
#include "FollowWorker.h"
#include "UnfollowWorker.h"

// TODO
// add button to view current status -> (Create new window for that)
// ...MMm
//
class GtkTwitterfu : public Gtk::Window {
    public:
        GtkTwitterfu();
        virtual ~GtkTwitterfu();
        void addID(const Glib::ustring &id);       /* Add id to treeview */ 
        void removeID(const Glib::ustring &id);    /* Remove id from treeview */ 
        void setStatus(const Glib::ustring &text);
    private:
        void on_button_quit();              /* Just quit */ 
        void on_button_find_followers();    /* InputWorker get input */ 
        void on_button_find_following();    /* InputWorker get input */ 
        void on_button_start_following();   /* Start the FollowingWorker */ 
        void on_button_stop_following();    /* Stop the FollowingWorker thread if running */ 
        void on_button_start_unfollowing(); /* Start unfollowing */ 
        void on_button_stop_unfollowing();  /* Stop unfollowing */ 
        void on_checkbutton_description();
        void on_checkbutton_follow_ratio();
        void on_checkbutton_profile_picture();
        void on_checkbutton_protected_profile();
        void on_checkbutton_near_timezone();
        void find_followers();              /* Works after InputWorker sigdone to get followers */ 
        void find_following();              /* Works after InputWorker sigdone to get following */ 
        void followed_user();   // Called when FollowWorker is done.
                                // TODO When FollowWorker is done/stopped update this->user->followers && GtkTwitterfu.treeview model
        void unfollowed_user(); // Called when UnfollowWorker is done.
                                // TODO When unfollow worker is done/stopped update this->user->following,

        /* Buttons and labels */ 
        Gtk::Button button_find_followers;
        Gtk::Button button_find_following;
        Gtk::Button button_start_following;
        Gtk::Button button_stop_following;
        Gtk::Button button_start_unfollowing;
        Gtk::Button button_stop_unfollowing;
        Gtk::Button button_quit;
        /*
         *Gtk::CheckButton checkbutton_near_timezone;
         */
        Gtk::CheckButton checkbutton_description;
        Gtk::CheckButton checkbutton_follow_ratio;
        Gtk::CheckButton checkbutton_profile_picture;
        Gtk::CheckButton checkbutton_protected_profile;
        Gtk::CheckButton checkbutton_near_timezone;
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
        Gtk::ButtonBox checkbox;

        InputWorker *input;                /* The inputWorker to get work in another thread and not block : separate window */ 
        FollowWorker *follow_worker;       /* The FollowWorker to follow and report ..etc */ 
        UnfollowWorker *unfollow_worker;   /* The UnfollowWorker to unfollow and report ..etc */ 

        // twitterfu
        User *user;
        deque< string > users_to_follow;   /* The users to follow */ 
};
#endif
