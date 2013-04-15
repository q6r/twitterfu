#ifndef FILTERS_H_
#define FILTERS_H_
#include <iostream>
#include <string>
#include <deque>
#include <algorithm>
#include "User.h"

using namespace std;
class User;

/**
 * Represent a user filter
 */
class Filters {
        public:
                Filters(User *p);
                ~Filters();
               /**
                 * We filter users depending on 5 rules
                 * 1) have a profile picture +1
                 * 2) have description +1
                 * 3) not protected profile +1
                 * 4) following>follower or following:followers ratio 75% +1
                 * 5) Near our timezone by -4 or +4 timezones +1
                 * @param user the user object
                 * @param userid the user id to apply filters to it
                 * @return true if successfuly filtered
                 */
                bool mainFilter(string userid);
                bool getProfilePicture() const;
                bool getDescription() const;
                bool getProtectedProfile() const;
                bool getFollowRatio() const;
                bool getNearTimezone() const;
                void setProfilePicture(const bool &n);
                void setDescription(const bool &n);
                void setProtectedProfile(const bool &n);
                void setFollowRatio(const bool &n);
                void setNearTimezone(const bool &n);
        private:
                /**
                 * Check if user's timezone is near us by -4 or +4 timezones
                 * the timezones in the deque is pushed in order of their
                 * distance.
                 *
                 * @param user the user object
                 * @param timezones of the userid
                 * @return true if able to predict time zone
                 */
                bool predictTimezone(const string &timezone);
                bool isFollower(const string &userid) const;
                /** have profile picture */
                bool profilePicture;
                /** have description */
                bool description;
                /** is the profile protected */
                bool protectedProfile;
                /** falls in our follow ratio */
                bool followRatio;
                /** is near us */
                bool nearTimezone;
                /** User parent */
                User *parent;
};
#endif
