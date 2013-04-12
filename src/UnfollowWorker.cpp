#include "UnfollowWorker.h"

UnfollowWorker::UnfollowWorker(User *_user, deque<string> _ids) :
    user(_user),
    thread(0),
    ids(_ids),
    stop(false)
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
    while(true) {
        {
            Glib::Mutex::Lock lock(mutex);
            if(stop) break;
        }

        for(deque<string>::iterator it = ids.begin(); it != ids.end(); it++) {
            string id = *it;
            std::cout << "Unfollowing " << id << std::endl;
            if(this->user->unfollow(id) == false)
                failures++;
            else
                unfollowed++;
            if(this->stop == true) {
                std::cout << "Stop flag reached" << std::endl;
                break;
            }
        }
        sig_done();
        break;
    }
    std::cout << "failures : " << failures << " unfollowed : " << unfollowed << std::endl;
}
