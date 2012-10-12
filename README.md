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

FAQ
===

Will you add support for DH-BLOWFISH?
-------------------------------------

The DH-BLOWFISH mechanism is deprecated in Atheme, and not included in the IRCv3 SASL
specification and not certified by the IETF as a protocol-generic SASL mechanism.

So I have no plans for that.  You should just use STARTTLS instead to establish
encryption.

Will you try to upstream this in Anope?
---------------------------------------

LOL.

No, really, are you?
--------------------

LOL.

Really, really, are you?
------------------------

I strongly doubt the current maintainer of Anope would accept the patch due to
political reasons.  So I offer this module instead.  Someone else can try to upstream
the functionality, but they should do it properly instead of implementing it as this
module.

Properly would be to farm out the `SASLImplementation` class to the protocol modules,
for example.

Does this really work?
----------------------

Yes!  I have also formally-proven my code by hand, but I have only done very light
testing on the code.  It works for me, and is formally proven though.  Your mileage,
may of course, vary greatly.

