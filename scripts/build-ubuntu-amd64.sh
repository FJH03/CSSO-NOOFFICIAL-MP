#!/bin/sh

sudo apt-get update
sudo apt-get install -f -y libopenal-dev g++-multilib gcc-multilib libpng-dev libjpeg-dev libfreetype6-dev libfontconfig1-dev libcurl4-gnutls-dev libsdl2-dev zlib1g-dev libbz2-dev libedit-dev
chmod +x waf
./waf configure -T release --build-games=csso --disable-warns --prefix="./output" $* &&
./waf install
