#ifndef FILTERS_H_
#define FILTERS_H_
#include <iostream>

using namespace std;

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
