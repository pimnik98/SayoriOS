FROM debian:11

RUN apt-get update
RUN apt-get install git -y
RUN apt-get install make -y
RUN apt-get install clang-13 -y
RUN apt-get install g++ -y
RUN apt-get install grub-pc-bin -y
RUN apt-get install xorriso -y
RUN git clone https://github.com/NDRAEY/SayoriOS.git /SayoriOS --depth 1

WORKDIR /SayoriOS

CMD ["sh", "-c", "git pull && make -j4 && make geniso && cp kernel.iso /output/"]

VOLUME /output
