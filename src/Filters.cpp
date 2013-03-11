#include "Filters.h"

Filters::Filters() {
}

Filters::~Filters() {
}

void Filters::setProfilePicture(bool n) {
        profilePicture = n;
}

void Filters::setDescription(bool n) {
        description = n;
}

void Filters::setProtectedProfile(bool n) {
        protectedProfile = n;
}

void Filters::setFollowRatio(bool n) {
        followRatio = n;
}

void Filters::setNearTimezone(bool n) {
        nearTimezone = n;
}

bool Filters::getProfilePicture() {
        return profilePicture;
}

bool Filters::getDescription() {
        return description;
}

bool Filters::getProtectedProfile() {
        return protectedProfile;
}

bool Filters::getFollowRatio() {
        return followRatio;
}

bool Filters::getNearTimezone() {
        return nearTimezone;
}
