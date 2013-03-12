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
#include <vector>
#include "sqlite3pp.h"
#include "database.h"
#include "User.h"

using namespace std;
class User;

/**
 * This will take src and add it to dest
 * @param dest the destination vector
 * @param src the source vector
 */
template < class T > void concatVectors(vector < T > &dest,
					vector < T > src);
/**
 * Takes a filename opens it get's the content
 * convert it to a vector and
 * @param filename the filename
 * @return vector.
 */
vector < string > fileToVector(string filename);
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
/**
 * Check if file exists
 * @param filename the filename
 * @return true if file exists
 */
bool fileExists(string filename);
/**
 * return a random number between from and to
 * @param from the from number
 * @param to the to number
 * @return number between [from,two]
 */
int randomize(int from, int to);
/**
 * Back to the first line and erase n characters
 * @param n number of blanks
 */
void cleanLine(int n);
#endif
