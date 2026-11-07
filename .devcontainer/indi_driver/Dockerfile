FROM ubuntu:24.04


RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    wget \
    vim \
    cmake \
    joe



# Indilib dependencies
RUN apt-get install -y libev-dev libcfitsio-dev libnova-dev libcurl4-openssl-dev libgsl-dev libfftw3-dev zlib1g-dev libusb-1.0-0-dev libjpeg-turbo8-dev

RUN apt-get install -y libtool autoconf pkg-config

# Indilib installation
RUN mkdir /deps

ARG INDI_VERSION=v2.2.2

RUN git clone --depth 1 -b ${INDI_VERSION} https://github.com/indilib/indi /deps/indi

RUN mkdir /build

RUN mkdir /build/indi
RUN cd /build/indi && cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr /deps/indi
RUN cd /build/indi && make -j$(nproc) && make install
