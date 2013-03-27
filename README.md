# twitterfu
Automated twitter follower and unfollower. It will use your twitter account and help you get more followers. Twitterfu will get a list of users to follow and each user on that list will be passed to the filters to decide whether to follow that user or not. This tool uses the API and is restricted by the API limits.

### Dependencies
- [Boost](http://boost.org)
- [Twitcurl](http://code.google.com/p/twitcurl/) 
 Included in this code, with twitter oauth api patch
- [Sqlite3](http://sqlite.org)

### Compile
Just run `./build.sh` and it will compile twitcurl and twitterfu then `./src/twitterfu`

### Configure
When you run the application `./src/twitterfu` for the first time you will be asked to enter
your password and username then a link to authorize the application using a PIN. (oauth)

Database is a sqlite3 at `$HOME/.twitterfu.sql`
