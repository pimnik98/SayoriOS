FROM debian:11

RUN apt-get update
RUN apt-get install git make clang-13 -y
RUN git clone https://github.com/NDRAEY/SayoriOS.git /SayoriOS --depth 1
RUN ln -s /usr/bin/clang-13 /usr/bin/cc

WORKDIR /SayoriOS

RUN make -j8

CMD ["cp", "kernel.iso", "/output/"]

VOLUME /output
