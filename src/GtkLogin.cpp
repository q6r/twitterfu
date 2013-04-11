#include "GtkLogin.h"

/******** Login class : */
GtkLogin::GtkLogin(string *_username, string *_password, Glib::ustring _status) :
    username(_username),
    password(_password),
    vbox(Gtk::ORIENTATION_VERTICAL),
    hbox(Gtk::ORIENTATION_HORIZONTAL),
    label_status(_status),
    button_okay("Okay"),
    button_quit("Quit")
{
    this->set_title("Login");
    this->set_size_request(200, 100);


    this->add(vbox);

    this->vbox.pack_start(entry_username, Gtk::PACK_SHRINK);
    this->vbox.pack_start(entry_password, Gtk::PACK_SHRINK);
    this->vbox.pack_start(hbox, Gtk::PACK_SHRINK);

    this->hbox.pack_start(button_okay, Gtk::PACK_SHRINK);
    this->hbox.pack_start(button_quit, Gtk::PACK_SHRINK);
    this->hbox.pack_start(label_status, Gtk::PACK_SHRINK);

    this->button_okay.signal_clicked().connect(
            sigc::mem_fun(*this, &GtkLogin::on_button_okay));
    this->button_quit.signal_clicked().connect(
            sigc::mem_fun(*this, &GtkLogin::on_button_quit));

    this->entry_password.set_visibility(false);

    this->show_all_children();
}

void GtkLogin::on_button_quit() {
    this->hide();
    exit(1); // TODO something cleaner
}

void GtkLogin::on_button_okay() {
    *username = entry_username.get_text();
    *password = entry_password.get_text();

    this->hide();
}

GtkLogin::~GtkLogin() {}


