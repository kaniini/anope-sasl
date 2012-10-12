anope-sasl
==========

SASL module for Anope 1.9.7 and later.

This is a from-scratch implementation of the SASL component of IRCv3 for Anope, based
in part (in terms of design) on the SASL stack in Atheme.  I wrote this in an afternoon,
as a proof of concept that yes, really, Anope *can* support SASL if they'd be willing to
take the 5 minutes (it took me 90 minutes, and I was unfamiliar with Anope's internals)
to actually write the code instead of spreading FUD.

For copying information, see `COPYING.md`.

Installation
============

Run the following command to build the module:

```
$ make ANOPE_SRC=/home/nenolod/dev-src/anope
g++ -Wall -fPIC -DPIC -shared -I/home/nenolod/dev-src/anope/include -I/home/nenolod/dev-src/anope/build/include -I/home/nenolod/dev-src/anope/modules/pseudoclients m_sasl.cpp -o m_sasl.so
```

`ANOPE_SRC` should be the path to your Anope source tree.

Now run this command to install:

```
$ make install ANOPE_BIN=/home/nenolod/services
Installed m_sasl.so.
```

Add `m_sasl.so` to your `services.conf` like any other module.  If your protocol module
is not supported, but you believe it should be, open a bug on the Github.  It will log
a message to the Anope logfile if there is not an implementation available.

*Note*: You will need to add `m_sasl.so` *after* your protocol module, so the module can
detect what protocol module is being used.
