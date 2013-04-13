#ifndef GTK_LOGIN_
#define GTK_LOGIN_

#include "gtkmm.h"
#include <string>

using namespace std;

class GtkLogin : public Gtk::Window {
    public:
        GtkLogin(string *_username, string *_password, const Glib::ustring &_status="");
        ~GtkLogin();
    private:
        void on_button_okay();
        void on_button_quit();
        Gtk::Button button_okay;
        Gtk::Button button_quit;
        Gtk::Entry entry_username;
        Gtk::Entry entry_password;
        Gtk::Label label_status;
        Gtk::Box vbox;
        Gtk::Box hbox;
        string *username; // these are from main ui
        string *password; // these are from main ui
};

#endif

