#ifndef UNFOLLOW_WORKER_
#define UNFOLLOW_WORKER_

#include "User.h"
#include <deque>
#include <string>
#include "gtkmm.h"

class GtkTwitterfu;

class UnfollowWorker {
    public:
        UnfollowWorker(User *_user, deque<string> _ids, GtkTwitterfu *_parent);
        ~UnfollowWorker();
        void start();
        void stopThread();
        Glib::Dispatcher sig_done;
    private:
        void run();
        Glib::Thread * thread;
        Glib::Mutex mutex;
        bool stop;
        deque<string> ids;
        GtkTwitterfu *parent;
        User *user;         /* This is a ptr to the object in gtktwitterfu */
};
#endif
