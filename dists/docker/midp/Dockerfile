FROM debian:jessie
MAINTAINER Emanuele Fornara "efornara@users.sourceforge.net"
RUN apt-get update
RUN apt-get install -y nano nvi make zip
RUN apt-get install -y python
RUN apt-get install -y openjdk-7-jdk
ADD scripts /opt
RUN chmod 755 /opt/get_midp
RUN /opt/get_midp
RUN dpkg --add-architecture i386
RUN apt-get update
RUN apt-get install -y libc6:i386
ENV PATH $PATH:/opt/apache-ant-1.9.6/bin
ENV WTK_HOME_DIRECTORY /opt/WTK
