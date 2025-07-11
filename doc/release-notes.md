Privora Core version 0.14.x is now available from:

  <https://privora.org/bin/privora-core-0.14.x/>

This is a new minor version release, including various bugfixes and
performance improvements.

Please report bugs using the issue tracker at github:

  <https://github.com/privora/privora/issues>

To receive security and update notifications, please subscribe to:

  <https://privoracore.org/en/list/announcements/join/>

Compatibility
==============

Privora Core is extensively tested on multiple operating systems using
the Linux kernel, macOS 10.8+, and Windows Vista and later.

Microsoft ended support for Windows XP on [April 8th, 2014](https://www.microsoft.com/en-us/WindowsForBusiness/end-of-xp-support),
No attempt is made to prevent installing or running the software on Windows XP, you
can still do so at your own risk but be aware that there are known instabilities and issues.
Please do not report issues about Windows XP to the issue tracker.

Privora Core should also work on most other Unix-like systems but is not
frequently tested on them.

From 0.13.1 onwards OS X 10.7 is no longer supported. 0.13.0 was intended to work on 10.7+, 
but severe issues with the libc++ version on 10.7.x keep it from running reliably. 
0.13.1 now requires 10.8+, and will communicate that to 10.7 users, rather than crashing unexpectedly.

Notable changes
===============


Detailed release notes follow. This overview includes changes that affect
behavior, not code moves, refactors and string updates. For convenience in locating
the code changes and accompanying discussion, both the pull request and
git merge commit are mentioned.

[to be filled in at release]

Credits
=======

Thanks to everyone who directly contributed to this release:

[to be filled in at release]

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/privora/).
=======
...

Known Bugs
==========

Since 0.14.0 the approximate transaction fee shown in Privora-Qt when using coin
control and smart fee estimation does not reflect any change in target from the
smart fee slider. It will only present an approximate fee calculated using the
default target. The fee calculated using the correct target is still applied to
the transaction and shown in the final send confirmation dialog.

0.14.x Change log
=================

...

Credits
=======

Thanks to everyone who directly contributed to this release:

...
