#!/bin/sh

sudo apt-get update
sudo apt-get install -y libbz2-dev
chmod +x waf
./waf configure -T release --sanitize=address,undefined --disable-warns --tests --prefix=out/ $* &&
./waf install &&
cd out &&
LD_LIBRARY_PATH=bin/ ./unittest
