#ifndef GTK_TWITTERFU_
#define GTK_TWITTERFU_

#include "gtkmm.h"
#include <iostream>
#include <cstring>
#include <ctime>

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

class GtkTwitterfu : public Gtk::Window {
    public:
        GtkTwitterfu();
        virtual ~GtkTwitterfu();
        void addUser(Glib::ustring username, Glib::ustring id);
    private:
        void on_button_quit();
        void on_button_find_followers();
        void on_button_find_following();
        void on_button_start_following();
        void on_button_stop_following();
        void get_target();
        void find_followers();
        void find_following();
        /*
         *void start_following();
         *void stop_following();
         */
        Gtk::Button button_find_followers;
        Gtk::Button button_find_following;
        Gtk::Button button_start_following;
        Gtk::Button button_stop_following;
        Gtk::Button button_quit;

        // treeview model
        class ModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                ModelColumns();
                Gtk::TreeModelColumn<Glib::ustring> m_col_text;
                Gtk::TreeModelColumn<Glib::ustring> m_col_number;
        };
        ModelColumns columns; // end of tv model

        //child widgets
        Gtk::Box vbox;
        Gtk::TreeView treeview;
        Gtk::ScrolledWindow scrolledwindow;

        Glib::RefPtr< Gtk::ListStore> refTreeModel;
        Gtk::ButtonBox buttonbox;

        InputWorker *input;
};
#endif
