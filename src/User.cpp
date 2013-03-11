#include "User.h"

User::User() {
}

User::~User() {
}

void User::setUsername(string n) {
        username = n;
}

void User::setPassword(string n) {
        password = n;
}

void User::setConsumerKey(string n) {
        consumer_key = n;
}

void User::setConsumerSecret(string n) {
        consumer_secret = n;
}

void User::setAccessTokenKey(string n) {
        access_token_key = n;
}

void User::setAccessTokenSecret(string n) {
        access_token_secret = n;
}

void User::setDBname(string n) {
        db_name = n;
}

void User::setTimezone(string n) {
        timezone = n;
}

void User::setFollowers(string n) {
        followers = n;
}

void User::setFollowing(string n) {
        following = n;        
}

string & User::getUsername() {
        return username;
}

string & User::getPassword() {
        return password;
}

string & User::getConsumerKey() {
        return consumer_key;
}

string & User::getConsumerSecret() {
        return consumer_secret;
}

string & User::getAccessTokenKey() {
        return access_token_key;
}

string & User::getAccessTokenSecret() {
        return access_token_secret;
}

string & User::getDBname() {
        return db_name;
}

string & User::getTimezone() {
        return timezone;
}

string & User::getFollowers() {
        return followers;
}

string & User::getFollowing() {
        return following;
}
