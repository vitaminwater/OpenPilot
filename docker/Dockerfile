FROM debian:latest

MAINTAINER Constantin Clauzel <constantin.clauzel@gmail.com>

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install curl -y --force-yes && \
    apt-get install build-essential gdb wget debhelper p7zip-full unzip flex bison -y --force-yes  && \
    apt-get install libsdl1.2-dev -y --force-yes  && \
    apt-get install libudev-dev -y --force-yes  && \
    apt-get install libusb-1.0-0-dev -y --force-yes  && \
    apt-get install libc6-i386 -y --yes && \
    apt-get install libxrender-dev -y --force-yes && \
    apt-get install libfontconfig-dev -y --force-yes && \
    useradd -ms /bin/bash builder

ADD bin/start_dev.sh /usr/local/bin/start_dev.sh

CMD ["/usr/local/bin/start_dev.sh"]
