#ifndef TWITTERFU_H_
#define TWITTERFU_H_
/**
 * The main part of twitterfu
 * @file twitterfu.h
 */
#include <iostream>
#include <pwd.h>
#include "User.h"
#include "GtkTwitterfu.h"

using namespace std;

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
