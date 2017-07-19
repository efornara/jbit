FROM debian:stretch
MAINTAINER Emanuele Fornara "efornara@users.sourceforge.net"
RUN apt-get update && apt-get install -y nano nvi wget zip bzip2 p7zip make
RUN apt-get install -y python3-pil
ADD scripts /opt
RUN /opt/get_djgpp ; /opt/get_watcom
