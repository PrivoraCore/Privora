# This is a Dockerfile for privorad.
FROM debian:bullseye as build-image

# Install required system packages
RUN apt-get update && apt-get install -y \
    automake \
    bsdmainutils \
    curl \
    g++ \
    libtool \
    make \
    pkg-config \
    patch 

# Build Privora
COPY . /tmp/privora/

WORKDIR /tmp/privora

RUN cd depends && \
    NO_QT=true make HOST=$(uname -m)-linux-gnu -j$(nproc)

RUN ./autogen.sh && \
    ./configure --without-gui --enable-tests --prefix=/tmp/privora/depends/$(uname -m)-linux-gnu && \
    make -j$(nproc) && \
    make check && \
    make install

# extract shared dependencies of privorad and privora-cli
# copy relevant binaries to /usr/bin, the COPY --from cannot use $(uname -m) variable in argument
RUN mkdir /tmp/ldd && \
    ./depends/ldd_copy.sh -b "./depends/$(uname -m)-linux-gnu/bin/privorad" -t "/tmp/ldd" && \
    ./depends/ldd_copy.sh -b "./depends/$(uname -m)-linux-gnu/bin/privora-cli" -t "/tmp/ldd" && \
    cp ./depends/$(uname -m)-linux-gnu/bin/* /usr/bin/

FROM debian:bullseye-slim

COPY --from=build-image /usr/bin/privorad /usr/bin/privorad
COPY --from=build-image /usr/bin/privora-cli /usr/bin/privora-cli
COPY --from=build-image /tmp/ldd /tmp/ldd

# restore ldd files in correct paths
# -n will not override libraries already present in this image,
# standard libraries like `libc` can crash when overriden at runtime
RUN cp -vnrT /tmp/ldd / && \
    rm -rf /tmp/ldd && \
    ldd /usr/bin/privorad

# Create user to run daemon
RUN useradd -m -U privorad
USER privorad

RUN mkdir /home/privorad/.privora
VOLUME [ "/home/privorad/.privora" ]

# Main network ports
EXPOSE 8168
EXPOSE 8888

# Test network ports
EXPOSE 18168
EXPOSE 18888

# Regression test network ports
EXPOSE 18444
EXPOSE 28888

ENTRYPOINT ["/usr/bin/privorad", "-printtoconsole"]

