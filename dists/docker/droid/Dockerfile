FROM debian:jessie
MAINTAINER Emanuele Fornara "efornara@users.sourceforge.net"
RUN apt-get update
RUN apt-get install -y nano nvi make zip
ADD scripts /opt
RUN . /opt/scripts/get_droid
ENV JBITANDROID_WORKDIR /opt
