#include "FollowWorker.h"

FollowWorker::FollowWorker(User *_user, deque<string> _ids) :
    thread(0),
    stop(false),
    ids(_ids),
    user(_user)
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

    while(true) {
        {
            Glib::Mutex::Lock lock (mutex);
            if(stop) break;
        }

        for(deque<string>::iterator it = ids.begin(); it != ids.end(); it++) {
            string id = *it;
            if(this->user->follow(id) == false)
                failures++;
            else
                followed++;
            // if we read stop flag!
            if(this->stop == true) {
                std::cout << "Stop flag found break!" << std::endl;
                break;
            }
        }

        sig_done();
        break;
    }

    std::cout << "failures : " << failures << " followed : " << followed << std::endl;

}

