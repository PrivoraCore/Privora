# Privora Core
*Note: some of the documents in this repository might still refer to Privora*

## Setup

[Privora Core] (https://privora.org/get-privora/download/) is the original Privora client and it builds the backbone of the network. However, it downloads and stores the entire history of Privora transactions (which is currently several hundreds MBs); depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.


## Building

The following are developer notes on how to build Privora on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Privora repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://dev.visucore.com/privora/doxygen/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Travis CI](travis-ci.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources
* Discuss on the [PrivoraTalk](https://privoratalk.org/) forums, in the [Development & Technical Discussion board](https://privoratalk.org/index.php?board=6.0).
* Discuss project-specific development on #privora-core-dev on Freenode. If you don't have an IRC client use [webchat here](http://webchat.freenode.net/?channels=privora-core-dev).
* Discuss general Privora development on #privora-dev on Freenode. If you don't have an IRC client use [webchat here](http://webchat.freenode.net/?channels=privora-dev).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
