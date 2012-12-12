Author : qnix <qnix@0x80.org>

twitterfu: A twitter follow/unfollow

Deps
====
- boost
- twitcurl (http://code.google.com/p/twitcurl/)

Configure
=========

cp twitter.conf.org twitter.conf and edit the file include these

- Username
- Password (not needed, we use Oauth; this is for later)
- Consumer key
- Consumer secret
- Access key
- Access secret

These can be obtained by creating an app for more info check dev.twitter.com

Compile
=======
- make && ./twitterfu
        

