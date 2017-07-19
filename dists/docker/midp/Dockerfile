FROM debian:stretch
MAINTAINER Emanuele Fornara "efornara@users.sourceforge.net"
RUN apt-get update && apt-get install -y nano nvi wget zip bzip2 p7zip make
RUN apt-get install -y python3-pil
RUN dpkg --add-architecture i386 && apt-get update \
 && apt-get install -y libc6:i386 && apt-get install -y openjdk-8-jdk
ADD scripts /opt
RUN /opt/get_midp
ENV PATH $PATH:/opt/apache-ant-1.9.9/bin
ENV WTK_HOME_DIRECTORY /opt/WTK
