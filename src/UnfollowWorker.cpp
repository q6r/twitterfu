#include "UnfollowWorker.h"
#include "GtkTwitterfu.h"

UnfollowWorker::UnfollowWorker(User *_user, const deque<string> &_ids, GtkTwitterfu *_parent) :
    user(_user),
    thread(0),
    ids(_ids),
    stop(false),
    parent(_parent)
{
    
}

UnfollowWorker::~UnfollowWorker() {
    {
        Glib::Mutex::Lock lock(mutex);
        stop = true;
    }

    if(thread)
        thread->join();
}

void UnfollowWorker::start() {
    thread = Glib::Thread::create( sigc::mem_fun(*this, &UnfollowWorker::run), true);
}

void UnfollowWorker::stopThread() {
    stop = true;
}

void UnfollowWorker::run() {
    int failures   = 0;
    int unfollowed = 0;

    this->parent->setStatus(  "Started Unfollowing...");

    while(true) {
        {
            Glib::Mutex::Lock lock(mutex);
            if(stop) break;
        }

        for(deque<string>::iterator it = ids.begin(); it != ids.end(); it++) {
            string id = *it;
            std::cout << "Unfollowing " << id << std::endl;
            if(this->user->unfollow(id) == false) {
                this->parent->setStatus(  "Unable to unfollow " + id);
                failures++;
            } else {
                this->parent->setStatus(  "Unfollowed " + id);
                unfollowed++;
            }
            if(this->stop == true) {
                break;
            }
        }
        sig_done();
        break;
    }

    this->parent->setStatus(  "Stopped Unfollowing. failures : " + Glib::ustring(to_string(failures)) + " unfollowed : " + Glib::ustring(to_string(unfollowed)));
}
