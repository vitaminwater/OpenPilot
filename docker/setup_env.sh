#!/bin/bash

echo 'deb http://http.debian.net/debian wheezy-backports main' >> /etc/apt/sources.list

apt-get update
apt-get upgrade -y --force-yes
apt-get install -y --force-yes curl wheezy-backports linux-image-amd64
curl -sSL https://get.docker.com/ | sh

