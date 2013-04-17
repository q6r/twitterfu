# twitterfu
Automated twitter follower and unfollower. It will use your twitter account and help you get more followers. Twitterfu will get a list of users to follow and each user on that list will be passed to the filters to decide whether to follow that user or not. This tool uses the API and is restricted by the API limits. It's a GUI

### Dependencies
- [Gtkmm](http://www.gtkmm.org/en/)
- [Gtk](http://www.gtk.org/)
- [Boost](http://boost.org)
- [Twitcurl](http://code.google.com/p/twitcurl/) 
 Included in this code, with twitter oauth api patch

### Compile
Type `cmake .` Then `make` that will build libtwitcurl and twitterfu and link it and all. The binary will be
at `src/twitterfu` if things went fine, you can install it by typing `make install`
