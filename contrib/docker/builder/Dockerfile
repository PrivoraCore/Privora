FROM ubuntu:20.04

# install required packages
RUN apt-get update && apt-get install -y software-properties-common

RUN apt-get update && apt-get install -y \
    autoconf automake bsdmainutils ccache cmake curl g++ g++-mingw-w64-x86-64 gcc gcc-mingw-w64-x86-64 git \
    libbz2-dev libtool make pkg-config python3-pip python3-zmq build-essential minizip lcov default-jre bison

# update mingw alternatives
RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
RUN update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

RUN pip3 install ez_setup

# create user to use
RUN useradd -m -U privora-builder

# create a volume for home directory
VOLUME [ "/home/privora-builder" ]

# start shell with created user
USER privora-builder
WORKDIR /home/privora-builder
