# Privora

[![Financial Contributors on Open Collective](https://opencollective.com/privora/all/badge.svg?label=financial+contributors)](https://opencollective.com/privora) [![latest-release](https://img.shields.io/github/release/privoraorg/privora)](https://github.com/privoraorg/privora/releases)
[![GitHub last-release](https://img.shields.io/github/release-date/privoraorg/privora)](https://github.com/privoraorg/privora/releases)
[![GitHub downloads](https://img.shields.io/github/downloads/privoraorg/privora/total)](https://github.com/privoraorg/privora/releases)
[![GitHub commits-since-last-version](https://img.shields.io/github/commits-since/privoraorg/privora/latest/master)](https://github.com/privoraorg/privora/graphs/commit-activity)
[![GitHub commits-per-month](https://img.shields.io/github/commit-activity/m/privoraorg/privora)](https://github.com/privoraorg/privora/graphs/code-frequency)
[![GitHub last-commit](https://img.shields.io/github/last-commit/privoraorg/privora)](https://github.com/privoraorg/privora/commits/master)

[Privora](https://privora.org) formerly known as Zcoin, is a privacy focused cryptocurrency that utilizes the [Lelantus Spark protocol](https://eprint.iacr.org/2021/1173) which supports high anonymity sets without requiring trusted setup and relying on standard cryptographic assumptions.

The Lelantus Spark cryptographic library and implementation was audited by [HashCloak](https://privora.org/about/research/papers/lelantus_spark_code_audit_report.pdf). The Lelantus Spark cryptography paper has undergone two separate audits by [HashCloak](https://privora.org/about/research/papers/Lelantus_Spark_Audit_Report.pdf) and [Daniel (Linfeng) Zhao](https://privora.org/about/research/papers/LinfengSparkAudit.pdf).

Privora also utilises [Dandelion++](https://arxiv.org/abs/1805.11060) to obscure the originating IP of transactions without relying on any external services such as Tor/i2P.

Privora uses a hybrid PoW and LLMQ Chainlocks system combining fair distribution of supply with protection against 51% attacks and quick finality of blocks and transactions. PrivoraPOW (a ProgPOW variant) is used as its Proof-of-Work algorithm which targets GPUs and is FPGA/ASIC resistant.

# Running with Docker

If you are already familiar with Docker, then running Privora with Docker might be the the easier method for you. To run Privora using this method, first install [Docker](https://store.docker.com/search?type=edition&offering=community). After this you may
continue with the following instructions.

Please note that we currently don't support the GUI when running with Docker. Therefore, you can only use RPC (via HTTP or the `privora-cli` utility) to interact with Privora via this method.

## Local Dockerfile

This repository contains a Dockerfile that you can build and run locally.

To build it, run from the root of this repository:

```sh
docker build . -t privora-local
```

After the process is completed, run a container based on the `privora-local` image you built:

```sh
docker run -d --name privorad -v "${HOME}/.privora:/home/privorad/.privora" privora-local
```

This will start a detached docker container, which you can interact with using `docker exec`. See the section "Interact with the container" for a list of useful commands you can use to manage your node. Make sure to change `privorad` with `privora-local`, if you have built the local Docker image.

## Docker image on DockerHub

If it doesn't already exist, create a `.privora` folder in your home (this is a workaround until [#1241](https://github.com/privoraorg/privora/issues/1241) is resolved):

```sh
mkdir -p ${HOME}/.privora
```

Pull our [latest official Docker image](https://hub.docker.com/r/privoraorg/privorad):

```sh
docker pull privoraorg/privorad
```

Start Privora daemon:

```sh
docker run -d --name privorad -v "${HOME}/.privora:/home/privorad/.privora" privoraorg/privorad
```

## Interact with the container

View current block count (this might take a while since the daemon needs to find other nodes and download blocks first):

```sh
docker exec privorad privora-cli getblockcount
```

View connected nodes:

```sh
docker exec privorad privora-cli getpeerinfo
```

Stop daemon:

```sh
docker stop privorad
```

Backup wallet:

```sh
docker cp privorad:/home/privorad/.privora/wallet.dat .
```

Start daemon again:

```sh
docker start privorad
```

# Linux Build Instructions and Notes

Privora contains build scripts for its dependencies to ensure all component versions are compatible. For additional options
such as cross compilation, read the [depends instructions](depends/README.md)

Alternatively, you can build dependencies manually. See the full [unix build instructions](doc/build-unix.md).

Bootstrappable builds can [be achieved with Guix.](contrib/guix/README.md)

## Development Dependencies (compiler and build tools)

- Debian/Ubuntu/Mint (minimum Ubuntu 18.04):

```sh
sudo apt-get update
sudo apt-get install python; sudo apt-get install git curl build-essential libtool automake pkg-config cmake
# Also needed for GUI wallet only:
sudo apt-get install qttools5-dev qttools5-dev-tools libxcb-xkb-dev bison
```

If you use a later version of Ubuntu, you may need to replace `python` with `python3`.

- Redhat/Fedora:

```sh
sudo dnf update
sudo dnf install bzip2 perl-lib perl-FindBin gcc-c++ libtool make autoconf automake cmake patch which
# Also needed for GUI wallet only:
sudo dnf install qt5-qttools-devel qt5-qtbase-devel xz bison
sudo ln /usr/bin/bison /usr/bin/yacc
```
- Arch:

```sh
sudo pacman -Sy
sudo pacman -S git base-devel python cmake
```

## Build Privora

1.  Download the source:

```sh
git clone https://github.com/privoraorg/privora
cd privora
```

2.  Build dependencies and privora:

Headless (command-line only for servers etc.):

```sh
cd depends
NO_QT=true make -j`nproc`
cd ..
./autogen.sh
./configure --prefix=`pwd`/depends/`depends/config.guess` --without-gui
make -j`nproc`
```

Or with GUI wallet as well:

```sh
cd depends
make -j`nproc`
cd ..
./autogen.sh
./configure --prefix=`pwd`/depends/`depends/config.guess`
make -j`nproc`
```

3.  *(optional)* It is recommended to build and run the unit tests:

```sh
./configure --prefix=`pwd`/depends/`depends/config.guess` --enable-tests
make check
```

If the build succeeded, two binaries will be generated in `/src`: `privorad` and `privora-cli`. If you chose to build the GUI, `privora-qt` will be also generated in the `qt` folder.

## macOS Build Instructions and Notes

See [doc/build-macos.md](doc/build-macos.md) for instructions on building on macOS.

*Note: these instructions are inherited from Privora and might not work as expected*

## Windows (64/32 bit) Build Instructions and Notes

See [doc/build-windows.md](doc/build-windows.md) for instructions on building on Windows 64/32 bit.

*Note: these instructions are inherited from Privora and might not work as expected*

# Run Privora

Now that you have your self-built or precompiled binaries, it's time to run Privora! Depending by your skill level and/or setup, you might want to use the command line tool or the graphic user interface. If you have problems or need support, [contact the community](https://privora.org/community/social/).

# Contributors

## Code Contributors

This project exists thanks to all the people who contribute. Would you like to help Privora and contribute? See the [[CONTRIBUTING](CONTRIBUTING.md)] file.
<a href="https://github.com/privoraorg/privora/graphs/contributors"><img src="https://opencollective.com/privora/contributors.svg?width=890&button=false" /></a>

## Financial Contributors

Become a financial contributor and help us sustain our community. [[Contribute](https://opencollective.com/privora/contribute)]

You can also donate to the [MAGIC Privora Fund](https://magicgrants.org/funds/privora/) which allows some US entities to claim a charitable deduction.

## Individuals

<a href="https://opencollective.com/privora"><img src="https://opencollective.com/privora/individuals.svg?width=890"></a>

## Organizations

Support this project with your organization. Your logo will show up here with a link to your website. [[Contribute](https://opencollective.com/privora/contribute)]

<a href="https://opencollective.com/privora/organization/0/website"><img src="https://opencollective.com/privora/organization/0/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/1/website"><img src="https://opencollective.com/privora/organization/1/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/2/website"><img src="https://opencollective.com/privora/organization/2/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/3/website"><img src="https://opencollective.com/privora/organization/3/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/4/website"><img src="https://opencollective.com/privora/organization/4/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/5/website"><img src="https://opencollective.com/privora/organization/5/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/6/website"><img src="https://opencollective.com/privora/organization/6/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/7/website"><img src="https://opencollective.com/privora/organization/7/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/8/website"><img src="https://opencollective.com/privora/organization/8/avatar.svg"></a>
<a href="https://opencollective.com/privora/organization/9/website"><img src="https://opencollective.com/privora/organization/9/avatar.svg"></a>
