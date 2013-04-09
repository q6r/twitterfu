#include "GtkTwitterfu.h"

GtkTwitterfu::GtkTwitterfu() :
    vbox(Gtk::ORIENTATION_VERTICAL),
    button_find_followers("Find followers"),
    button_find_following("Find following"),
    button_start_following("Start following"),
    button_stop_following("Stop following"),
    button_quit("Quit")
{

    this->set_title("Twitterfu");
    this->set_border_width(10);
    this->set_default_size(400,200);

    this->add(vbox);

    // scrolled if necessary for treeview
    scrolledwindow.add(treeview);
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // add to vbox
    vbox.pack_start(scrolledwindow);
    vbox.pack_start(buttonbox, Gtk::PACK_SHRINK);

    // add button to buttonbox
    buttonbox.pack_start(button_find_followers, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_find_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_start_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_stop_following, Gtk::PACK_SHRINK);
    buttonbox.pack_start(button_quit, Gtk::PACK_SHRINK);

    buttonbox.set_layout(Gtk::BUTTONBOX_END);

    /* create content of tree view */
    refTreeModel = Gtk::ListStore::create(columns);
    treeview.set_model(refTreeModel);

    Gtk::TreeModel::Row row = *(refTreeModel->append());
    
    row[columns.m_col_text]  = "qnix";
    row[columns.m_col_number] = "12345";

    row = *(refTreeModel->append());
    row[columns.m_col_text]  = "admin";
    row[columns.m_col_number] = "9999";

    row = *(refTreeModel->append());
    row[columns.m_col_text]  = "test";
    row[columns.m_col_number] = "12344";


    // add the treeview's view columns:
    // this will be shown
    treeview.append_column("Username", columns.m_col_text);
    treeview.append_column("ID", columns.m_col_number);



    // create button singals()
    button_quit.signal_clicked().connect( sigc::mem_fun(*this,
                &GtkTwitterfu::on_button_quit));
    button_find_followers.signal_clicked().connect(
            sigc::mem_fun(*this,
                &GtkTwitterfu::on_button_find_followers));


    this->show_all_children();
}

void GtkTwitterfu::on_button_find_followers() {
}

void GtkTwitterfu::on_button_find_following() {
}

void GtkTwitterfu::on_button_start_following() {
}

void GtkTwitterfu::on_button_stop_following() {
}

void GtkTwitterfu::on_button_quit() {
    this->hide();
}

GtkTwitterfu::~GtkTwitterfu() {
}
