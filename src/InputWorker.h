#ifndef INPUT_WORKER_
#define INPUT_WORKER_

#include "gtkmm.h"
#include "GtkTwitterfuInput.h"

class InputWorker : public GtkTwitterfuInput {
    public:
        InputWorker(const Glib::ustring &username);
        ~InputWorker();
        void start();
        Glib::Dispatcher sig_done;
    protected:
        void run();
        Glib::Thread * thread;
        Glib::Mutex mutex;
        bool stop;
};
#endif
