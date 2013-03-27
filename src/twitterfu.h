#ifndef TWITTERFU_H_
#define TWITTERFU_H_
/**
 * The main part of twitterfu
 * @file twitterfu.h
 */
#include <iostream>
#include <curl/curl.h>
#include <twitcurl.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sys/stat.h>
#include <ctime>
#include <algorithm>
#include <deque>
#include "sqlite3pp.h"
#include "User.h"

using namespace std;
class User;

/**
 * Take user input from stdin
 * @return selected option number
 */
int optionSelect();
/**
 * Do a specific job depending on the option
 * @param user the user object
 * @param opt the option number
 */
void optionParse(User * user, int opt);

/**
 * Show available option to the user
 */
void optionShow();
#endif
