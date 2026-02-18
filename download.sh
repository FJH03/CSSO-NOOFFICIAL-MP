#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}=== Супер загрузчик 3000 ===${NC}"

echo "Выберите файл:"
PS3="Введите номер: "
options_download=("Android-ndk" "Android-sdk" "Clang" "Всё" "Выход")

select download in "${options_download[@]}"
do
    case $download in
        "Android-ndk")   PLATFORM="ndk"; break ;;
        "Android-sdk") PLATFORM="sdk"; break ;;
        "Clang") PLATFORM="clang"; break ;;
        "Всё") PLATFORM="all"; break ;;
        "Выход")   exit 0 ;;
        *) echo "Неверный выбор $REPLY" ;;
    esac
done

echo -e "\n${GREEN}Идет загрузка файлов...${NC}"

if [[ "$PLATFORM" == "ndk" ]]; then
    rm -rf ../android-ndk-r10e
    wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip -O android-ndk-r10e-linux-x86_64.zip
    unzip android-ndk-r10e-linux-x86_64.zip
    mv android-ndk-r10e ../
    rm android-ndk-r10e-linux-x86_64.zip

elif [[ "$PLATFORM" == "sdk" ]]; then
    rm -rf ../android-sdk
    SDKTOOLS="https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip"
	SDKMANAGER="cmdline-tools/bin/sdkmanager --sdk_root=android-sdk"
    
    mkdir android-sdk || exit 1
    pushd android-sdk > /dev/null
    wget $SDKTOOLS -O sdktools.zip > /dev/null || exit 1
    unzip sdktools.zip || exit 1
    rm sdktools.zip || exit 1
    popd > /dev/null

    echo y | android-sdk/$SDKMANAGER --install "build-tools;33.0.1" "platform-tools" "platforms;android-29"
    mv android-sdk ../

elif [[ "$PLATFORM" == "clang" ]]; then
    rm -rf ../clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04
    wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
    echo -e "\n${GREEN}Распаковка файлов...${NC}"
    tar -xf clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
    mv clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04 ../
    rm clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz

elif [[ "$PLATFORM" == "all" ]]; then
    rm -rf ../android-sdk
    rm -rf ../android-ndk-r10e
    rm -rf ../clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04
    echo -e "\n${GREEN}Скачивание android-ndk...${NC}"
    wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip -O android-ndk-r10e-linux-x86_64.zip
    unzip android-ndk-r10e-linux-x86_64.zip
    mv android-ndk-r10e ../
    rm android-ndk-r10e-linux-x86_64.zip

    echo -e "\n${GREEN}Скачивание clang 11...${NC}"
    wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang11.tar.xz
    tar -xf clang11.tar.xz
    mv clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04 ../
    rm clang11.tar.xz

    echo -e "\n${GREEN}Скачивание android-sdk...${NC}"
    SDKTOOLS="https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip"
	SDKMANAGER="cmdline-tools/bin/sdkmanager --sdk_root=android-sdk"
    
    mkdir android-sdk || exit 1
    pushd android-sdk > /dev/null
    wget $SDKTOOLS -O sdktools.zip > /dev/null || exit 1
    unzip sdktools.zip || exit 1
    rm sdktools.zip || exit 1
    popd > /dev/null

    echo y | android-sdk/$SDKMANAGER --install "build-tools;33.0.1" "platform-tools" "platforms;android-29"
    mv android-sdk ../
fi

echo -e "\n${GREEN}Загрузка завершена${NC}"