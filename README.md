# Privora
[Privora](https://privora.org) is a privacy focused cryptocurrency that utilizes the [Lelantus Spark protocol](https://eprint.iacr.org/2021/1173) which supports high anonymity sets without requiring trusted setup and relying on standard cryptographic assumptions.

At Privora, we believe privacy isn't something you should struggle for - it should be part of every transaction by default. That's why we've created a next-generation mixer that puts real privacy back in your hands. Whether you're sending, receiving, or simply protecting your crypto, Privora makes the process secure, simple, and private - without requiring trust.

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
git clone https://github.com/PrivoraCore/Privora
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