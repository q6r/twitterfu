Author : qnix <qnix@0x80.org>

twitterfu: Automated twitter follow/unfollow

Deps
====
- boost
- twitcurl (http://code.google.com/p/twitcurl/)

Configure
=========

cp twitter.conf.org twitter.conf and edit the file include these

- Username
- Password
- (optional) Proxy address
- (optional) Proxy port
- (optional) Proxy Username
- (optional) Proxy Password

when you run the application for the first time you will be asked to visit
a twitter link and authorize the application by entering the PIN.

Compile
=======
- make && ./twitterfu
