#ifndef FILTERS_H_
#define FILTERS_H_
#include <iostream>
#include <string>
#include "twitterfu.h"

using namespace std;
class User;

/**
 * Represent a user filter
 */
class Filters {
        public:
                Filters();
                ~Filters();
                void setProfilePicture(bool n);
                void setDescription(bool n);
                void setProtectedProfile(bool n);
                void setFollowRatio(bool n);
                void setNearTimezone(bool n);

                bool getProfilePicture();
                bool getDescription();
                bool getProtectedProfile();
                bool getFollowRatio();
                bool getNearTimezone();
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
                bool mainFilter(User *user, string userid);
                /**
                 * Check if user's timezone is near us by -4 or +4 timezones
                 * the timezones in the vector is pushed in order of their
                 * distance.
                 *
                 * @param user the user object
                 * @param timezones of the userid
                 * @return true if able to predict time zone
                 */
                bool predictTimezone(User *user, string timezones);
                /**
                 * Show a list of filters and help
                 * in toggling them.
                 * @param user the user object
                 */
                void filterList();
        private:
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
};
#endif
