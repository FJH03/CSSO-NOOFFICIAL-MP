#!/bin/sh
export ANDROID_NDK_HOME=$PWD/android-ndk-r10e/
export PATH="$PWD/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH"
./waf configure -T release --build-game=cstrike --prefix=Srceng-mod-launcher/android --android=aarch64,host,21 --target=../aarch64 --enable-opus --togles --disable-warns &&
./waf install --target=client,server,engine,GameUI,vgui2,vguimatsurface,vstdlib --strip

if [ -e "Srceng-mod-launcher/android/lib/arm64-v8a/README.md" ]; then
	rm Srceng-mod-launcher/android/lib/arm64-v8a/README.md
fi
