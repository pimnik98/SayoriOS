FROM alpine:latest

RUN apk update
RUN apk add git
RUN apk add bash
RUN apk add make
RUN apk add clang
RUN apk add grub
RUN apk add xorriso
RUN git clone https://github.com/NDRAEY/SayoriOS.git /SayoriOS --depth 1

WORKDIR /SayoriOS

CMD ["sh", "-c", "git pull && make -j4 && make geniso && cp kernel.iso /output/"]

VOLUME /output
