#include "InputWorker.h"

InputWorker::InputWorker(const Glib::ustring &username) :
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
        if(this->getResult() != NULL) {
            sig_done(); // send the signal
            break;
        }

    }
}


