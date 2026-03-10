# CS:SO Android Source Code
Credits:
Ported by ndke, den4iklovelinux, cherrybtw
Mod author: PimonFeeD
Source engine port author: nillerusr
1.1 Updates: PowerSiderS

CS:SO Apk Source Code https://github.com/ndke01/csso-android-launcher


# Source Engine
[![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/build.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/build.yml) [![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml)
 Discord: [![Discord Server](https://img.shields.io/discord/672055862608658432.svg)](https://discord.gg/hZRB7WMgGw)
 

Information from [wikipedia](https://wikipedia.org/wiki/Source_(game_engine)):

Source is a 3D game engine developed by Valve.
It debuted as the successor to GoldSrc with Half-Life: Source in June 2004,
followed by Counter-Strike: Source and Half-Life 2 later that year.
Source does not have a concise version numbering scheme; instead, it was released in incremental versions

Source code is based on TF2 2018 leak. Don't use it for commercial purposes.

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

# Features:
- Android, OSX, FreeBSD, Windows, Linux( glibc, musl ) support
- Arm support( except windows )
- 64bit support
- Modern toolchains support
- Fixed many undefined behaviours
- Touch support( even on windows/linux/osx )
- VTF 7.5 support
- PBR support
- bsp v19-v21 support( bsp v21 support is partial, portal 2 and csgo maps works fine )
- mdl v46-49 support
- Removed useless/unnecessary dependencies
- Achivement system working without steam
- Fixed many bugs
- Serverbrowser works without steam
# CS:SO
- Bink video
- Lunasvg
- Ability to add your custom avatar & clantag. Added also UI implements for it
- Added Gyroscope (Dogshit Edition)
- Fixed Name & Avatar not displayed in Achievement Menu
- Now achievement menu save your stats , stuffs
- Decreased the issues of missing sounds in gameplay (use snd_restart if sounds began to disappear)
- Re-Worked on scoreboard (Avatars display and clantag)
- Fixed custom avatar not displayed on voice icon
- small optimizations lol
- CS2 Animation (Grenades Only)
- Skip HTML Panel to Choose Team Directly
- Agent Show Blind Animation
- Spam Inspection Animation
- i forgot. Maybe that's all?
# Bugs
- Menu does't show avatars (IDK why) sometimes show and sometimes no
- you need `jpg` and ratio 1:1. put avatar in `csso/materials/vgui/logos` otherwise, it will show you error message
- Backgrounds changing not supported currently (issue from the project itself?)

# Current tasks
- Rewrite materialsystem for OpenGL render
- dxvk-native support
- Elbrus port
- Bink audio support( for video_bink )

# How to Build?
- [Building instructions(EN)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-(EN))
- [Building instructions(RU)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-(RU))

# Support me
BTC: `bc1qnjq92jj9uqjtafcx2zvnwd48q89hgtd6w8a6na`

ETH: `0x5d0D561146Ed758D266E59B56e85Af0b03ABAF46`

XMR: `48iXvX61MU24m5VGc77rXQYKmoww3dZh6hn7mEwDaLVTfGhyBKq2teoPpeBq6xvqj4itsGh6EzNTzBty6ZDDevApCFNpsJ`
