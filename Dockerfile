FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install base dependencies (enable universe for Ruby)
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository universe \
    && apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    xz-utils \
    libncurses5 \
    libusb-1.0-0-dev \
    libudev-dev \
    pkg-config \
    openssh-client \
    ca-certificates \
    ruby-full \
    && rm -rf /var/lib/apt/lists/*

# Install Ceedling for unit tests
RUN gem install ceedling

# Versions matching your .stm32env
ARG ARM_GCC_VERSION=14.2.rel1
ARG OPENOCD_VERSION=0.12.0

# Install ARM GCC Toolchain
RUN mkdir -p /opt/toolchain && \
    cd /opt/toolchain && \
    wget -q "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_GCC_VERSION}/binrel/arm-gnu-toolchain-${ARM_GCC_VERSION}-x86_64-arm-none-eabi.tar.xz" -O gcc-arm.tar.xz && \
    tar -xf gcc-arm.tar.xz && \
    rm gcc-arm.tar.xz && \
    mv arm-gnu-toolchain-* arm-none-eabi-gcc

ENV PATH="/opt/toolchain/arm-none-eabi-gcc/bin:${PATH}"

# Install OpenOCD
RUN apt-get update && apt-get install -y \
    libhidapi-dev \
    libftdi-dev \
    libjaylink-dev \
    libgpiod-dev \
    autoconf \
    automake \
    libtool \
    texinfo \
    && rm -rf /var/lib/apt/lists/*

RUN cd /tmp && \
    git clone --depth 1 --branch v${OPENOCD_VERSION} https://github.com/openocd-org/openocd.git && \
    cd openocd && \
    ./bootstrap && \
    ./configure --enable-stlink --enable-jlink --enable-ftdi && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /tmp/openocd

ENV ARM_GCC_PATH=/opt/toolchain/arm-none-eabi-gcc/bin
ENV GCC_PATH=/opt/toolchain/arm-none-eabi-gcc/bin
ENV OPENOCD=/usr/local/bin/openocd

WORKDIR /workspace

CMD ["/bin/bash"]