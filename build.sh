#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}=== Супер компилер 3000 ===${NC}"

echo "Выберите платформу:"
PS3="Введите номер: "
options_os=("Linux" "Android" "Server" "Продолжить сборку" "Продолжить сборку(андроид)" "Гайд" "Выход")

select os in "${options_os[@]}"
do
    case $os in
        "Linux")   PLATFORM="linux"; break ;;
        "Android") PLATFORM="android"; break ;;
        "Server") PLATFORM="server"; break ;;
        "Продолжить сборку") PLATFORM="continuepc"; break ;;
        "Продолжить сборку(андроид)") PLATFORM="continue"; break ;;
        "Гайд") echo -e "Данная программа создана для упрощения компиляции и сборки апк.\n
Рекомендуется для сборки на андроид скачивать android ndk и sdk по скрипту download.sh
папку с исходниками апк поставьте рядом с исходниками движка игры переименовав в apk-sources
или же отредайктируйте скрипт заменив apk-sources на ваше название.\n
Для линукс билдов создайте рядом с движком папку games где будут все ваши игры, туда будут сохраняться
все скомпилированные либы и вы сможете оттуда сразу запускать игру.\n
Сервер билд вашей игры скрипт будет сохранять в вашей папке движка в папку out.\n
Автор скрипта ndke" ;;
    "Выход")   exit 0 ;;
        *) echo "Неверный выбор $REPLY" ;;
    esac
done

echo -e "\nВыберите битность:"
bit_build=("64" "32")

if [[ "$PLATFORM" != "android" && "$PLATFORM" != "continuepc" && "$PLATFORM" != "continue" ]]; then
    select build_pc_bit in "${bit_build[@]}"
    do
        case $build_pc_bit in
            "64")
                BIT_PC_FLAGS="" 
                break
                ;;
            "32")
                BIT_PC_FLAGS="--32bits"
                break
                ;;
            *) echo "Неверный выбор $REPLY" ;;
        esac
    done
elif [[ "$PLATFORM" != "continuepc" && "$PLATFORM" != "continue" ]]; then
    select build_bit in "${bit_build[@]}"
    do
        case $build_bit in
            "64")
                BIT_FLAGS="--android=aarch64,host,21" 
                break
                ;;
            "32")
                BIT_FLAGS="--android=armeabi-v7a-hard,host,21"
                break
                ;;
            *) echo "Неверный выбор $REPLY" ;;
        esac
    done
fi

echo -e "\nВыберите тип сборки:"
    options_type=("Release" "Debug")

select build_type in "${options_type[@]}"
do
    case $build_type in
        "Release")
            MODE="release"
            FINAL_FLAGS="--strip"
            APK_FLAGS="assembledebug"
            break
            ;;
        "Debug")
            MODE="debug"
            APK_FLAGS="assembledebug"
            break
            ;;
        *) echo "Неверный выбор $REPLY" ;;
    esac
done

echo -e "\nКонфигурация: ${GREEN}$os [$MODE]${NC}"
echo "----------------------------------------"

if [[ "$PLATFORM" == "linux" ]]; then
    rm -r build
    python3 ./waf configure -T $MODE --build-games=csso --prefix=../games/ --disable-warns --togles $BIT_PC_FLAGS

elif [[ "$PLATFORM" == "android" ]]; then
    export ANDROID_NDK_HOME="$(readlink -f ../android-ndk-r10e)"
    export PATH="$(readlink -f ../clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04/bin):$PATH"
    rm -r build
    python3 ./waf configure -T $MODE --build-games=csso --prefix=../apk-sources/app/src/main/ --disable-warns --togles $BIT_FLAGS

elif [[ "$PLATFORM" == "server" ]]; then
    rm -r build
    python3 ./waf configure -T $MODE --build-games=csso --prefix=out/ --disable-warns -d $BIT_PC_FLAGS

elif [[ "$PLATFORM" == "continue" ]]; then
    export ANDROID_NDK_HOME="$(readlink -f ../android-ndk-r10e)"
    export PATH="$(readlink -f ../clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04/bin):$PATH"
fi

echo -e "\n${GREEN}Запуск компиляции...${NC}"
python3 ./waf install -p $FINAL_FLAGS
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}Компиляция успешно завершена${NC}"
else
    echo -e "\n\033[0;31mОшибка: Компиляция завершилась ошибкой\033[0m"
    exit 1
fi

if [[ "$PLATFORM" == "android" || "$PLATFORM" == "continue" ]]; then
    echo -e "\n${GREEN}Сборка апк${NC}"
    cd ../apk-sources/app/src/main
    cp -a lib/. jniLibs/
    rm -rf lib
    cd ../../../
    export ANDROID_HOME="$(readlink -f ../android-sdk)"
    ./gradlew assemble $APK_FLAGS
    echo -e "\n${GREEN}Сборка апк завершена${NC}"
fi