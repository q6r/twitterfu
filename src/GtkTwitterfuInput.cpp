#include "GtkTwitterfuInput.h"

GtkTwitterfuInput::GtkTwitterfuInput(const Glib::ustring &text_info) : 
    button_okay("Ok"),
    vbox(Gtk::ORIENTATION_VERTICAL),
    result(NULL)
{

    this->set_title(text_info);
    this->set_size_request(200, 100);

    entry.set_max_length( 50 );
    entry.set_text("");

    // Create button_ok connection
    button_okay.signal_clicked().connect(
            sigc::mem_fun(*this, &GtkTwitterfuInput::on_button_okay));

    vbox.pack_start(entry, Gtk::PACK_SHRINK);
    vbox.pack_start(button_okay, Gtk::PACK_SHRINK);

    // button def.
    button_okay.set_can_default( true );
    button_okay.grab_default();

    this->add(vbox);
    this->show_all_children();
    this->show_all();
}

char * GtkTwitterfuInput::getResult() {
    return this->result;
}

void GtkTwitterfuInput::on_button_okay() {
    result = new char[128]();
    strncpy(result, this->entry.get_text().c_str(), 127);
    this->hide();
}

GtkTwitterfuInput::~GtkTwitterfuInput() {
}


