#!/bin/sh

wget -nv https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip -o /dev/null
unzip -q android-ndk-r10e-linux-x86_64.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r10e/
export NDK_HOME=$PWD/android-ndk-r10e/
wget -nv https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
mkdir -p $HOME/llvm11
tar -xJf clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz --strip-components=1 -C $HOME/llvm11
chmod +x waf
chmod +x $HOME/llvm11/bin/llvm-strip
export PATH=$HOME/llvm11/bin:$PATH
CFLAGS="-O2" CXXFLAGS="-O2" LDFLAGS="-s -flto" ./waf configure -T release --build-games=csso --togles --android=aarch64,host,21 --prefix=./output --disable-warns &&
./waf install