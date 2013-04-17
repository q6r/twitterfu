#include "FollowWorker.h"
#include "GtkTwitterfu.h"

FollowWorker::FollowWorker(User *_user, const deque<string> &_ids, GtkTwitterfu *_parent) :
    thread(0),
    stop(false),
    ids(_ids),
    user(_user),
    parent(_parent)
{ 

}

FollowWorker::~FollowWorker() { 
    { 
        Glib::Mutex::Lock lock(mutex);
        stop = true;
    }
    
    if(thread)
        thread->join();
}

void FollowWorker::start() {
    thread = Glib::Thread::create(sigc::mem_fun(*this, &FollowWorker::run), true);
}

void FollowWorker::stopThread() {
    stop = true;
}

void FollowWorker::run() {
    // run until we get input
    // TODO take it easy
    int failures = 0;
    int followed = 0;
    
    this->parent->setStatus( "Started following...");

    while(true) {
        {
            Glib::Mutex::Lock lock (mutex);
            if(stop) break;
        }

        for(deque<string>::iterator it = ids.begin(); it != ids.end(); it++) {
            string id = *it;
            string username;
            if(this->user->follow(id, username) == false) {
                this->parent->setStatus( "Ignored user id " + id);
                this->parent->removeID(id);
                failures++;
            } else {
                this->parent->setStatus( "Followed " + username);
                this->parent->removeID( id );
                followed++;
            }
            // if we read stop flag!
            if(this->stop == true) {
                break;
            }
        }

        sig_done();
        break;
    }

    this->parent->setStatus( "Stopped following. Failures : " + Glib::ustring(to_string(failures)) + " followed : " + Glib::ustring(to_string(followed)));
}

