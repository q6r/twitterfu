#ifndef FOLLOW_WORKER_
#define FOLLOW_WORKER_

#include "User.h"
#include <deque>
#include <string>
#include "gtkmm.h"

/*
 *Follow user worker this will have access to 
 *GtkTwitterfu so it can add/remove/change to the UI
 */
class FollowWorker {
    public:
        // TODO follow worker should also have access to the tree
        // model in GtkTwitterfu so it can update it of followed users
        // it will user GtkTwitterfu::removeID for each followed id
        FollowWorker(User *_user, deque<string> _id);
        /**
         * Start thread
         */
        void start();
        /**
         * Stop thread
         */
        void stopThread();
        ~FollowWorker();
        // Dispatcher when done, TODO Do we need one ?
        Glib::Dispatcher sig_done;
    private:
        void run();
        Glib::Thread * thread;
        Glib::Mutex mutex;
        bool stop;
        deque<string> ids;   /* IDs to follow */ 
        User *user;         /* This is a ptr to the object in gtktwitterfu */
};
#endif
