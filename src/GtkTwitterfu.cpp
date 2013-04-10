#include "GtkTwitterfu.h"

/******** Input class : this is derived by InputWorker for threading */
GtkTwitterfuInput::GtkTwitterfuInput(Glib::ustring text_info) : 
    button_okay("Ok"),
    vbox(Gtk::ORIENTATION_VERTICAL)
{
    result = new char[128](); // TODO user job to delete

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

/******** Worker : public Input */
InputWorker::InputWorker(Glib::ustring username) :
    thread(0),
    stop(false),
    GtkTwitterfuInput(username) { }

void InputWorker::start() {
    thread = Glib::Thread::create(sigc::mem_fun(*this, &InputWorker::run), true);
}

InputWorker::~InputWorker() {
    { 
        Glib::Mutex::Lock lock(mutex);
        stop = true;
    }
    
    if(thread)
        thread->join();
}

void InputWorker::run() {
    // run until we get input
    // TODO take it easy
    while(true) {
        {
            Glib::Mutex::Lock lock (mutex);
            if(stop) break;
        }
        // if button_ok was clicked and result has
        // something
        if(this->getResult()[0] != 0x00) {
            sig_done(); // send the signal
            break;
        }
    }
}

char * GtkTwitterfuInput::getResult() {
    return this->result;
}

void GtkTwitterfuInput::on_button_okay() {
    strncpy(result, this->entry.get_text().c_str(), 127);
    this->hide();
}

GtkTwitterfuInput::~GtkTwitterfuInput() {
}

/******** Main gtk class */
GtkTwitterfu::GtkTwitterfu() :
    vbox(Gtk::ORIENTATION_VERTICAL),
    button_find_followers("Find followers"),
    button_find_following("Find following"),
    button_start_following("Start following"),
    button_stop_following("Stop following"),
    button_quit("Quit"),
    input(NULL)
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

    // add the treeview's view columns:
    // this will be shown
    treeview.append_column("Username", columns.m_col_text);
    treeview.append_column("ID", columns.m_col_number);

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


    this->show_all_children();
}

GtkTwitterfu::ModelColumns::ModelColumns() {
    this->add(m_col_text);
    this->add(m_col_number);
}

void GtkTwitterfu::addUser(Glib::ustring username, Glib::ustring id) {
    refTreeModel              = Gtk::ListStore::create(columns);
    treeview.set_model(refTreeModel);
    Gtk::TreeModel::Row row   = *(refTreeModel->append());
    row[columns.m_col_text]   = username;
    row[columns.m_col_number] = id;
}

void GtkTwitterfu::find_followers() {
    std::cout << "Finding followers for " << input->getResult() << std::endl;
    delete input;
    input = NULL;
}

void GtkTwitterfu::find_following() {
    std::cout << "Finding following for " << input->getResult() << std::endl;
    delete input;
    input = NULL;
}

void GtkTwitterfu::on_button_find_followers() {
    
    // InputWorker still waiting for input.
    if(input != NULL)
        return;

    input = new InputWorker("Enter username");
    input->sig_done.connect( sigc::mem_fun(*this, &GtkTwitterfu::find_followers));
    input->start();
}

void GtkTwitterfu::on_button_find_following() {
    // Input worker still waiting for input.
    if(input != NULL)
        return;

    input = new InputWorker("Enter username");
    input->sig_done.connect( sigc::mem_fun(*this, &GtkTwitterfu::find_following));
    input->start();
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
