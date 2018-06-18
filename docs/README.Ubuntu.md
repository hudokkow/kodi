![Kodi Logo](resources/banner_slim.png)

# Debian/Ubuntu build guide
This guide has been tested with Ubuntu 16.04.4 x86_64 (Xenial), 17.10 (Artful) and 18.04 (Bionic). Please read it in full before you proceed to familiarize yourself with the build procedure.

Several other distributions have **[specific build guides](README.md)** and a general **[Linux build guide](README.Linux.md)** is also available.

## Table of Contents
1. **[Document conventions](#1-document-conventions)**
2. **[Get the source code](#2-get-the-source-code)**
3. **[Install the required packages](#3-install-the-required-packages)**  
  3.1. **[Get build dependencies automagically](#31-get-build-dependencies-automagically)**  
  3.2. **[Get build dependencies manually](#32-get-build-dependencies-manually)**  
  3.3. **[Build missing dependencies](#33-build-missing-dependencies)**
4. **[Build Kodi](#4-build-kodi)**

## 1. Document conventions
This guide assumes you are using `terminal`, also known as `console`, `command-line` or simply `cli`. Commands need to be run at the terminal, one at a time and in the provided order.

This is a comment that provides context:
```
this is a command
this is another command
and yet another one
```

**Example:** Clone Kodi's current master branch:
```
git clone https://github.com/xbmc/xbmc kodi
```

Commands that contain strings enclosed in angle brackets denote something you need to change to suit your needs.
```
git clone -b <branch-name> https://github.com/xbmc/xbmc kodi
```

**Example:** Clone Kodi's current Krypton branch:
```
git clone -b Krypton https://github.com/xbmc/xbmc kodi
```

Several different strategies are used to draw your attention to certain pieces of information. In order of how critical the information is, these items are marked as a note, tip, or warning. For example:
 
**NOTE:** Linux is user friendly... It's just very particular about who its friends are.  
**TIP:** Algorithm is what developers call code they do not want to explain.  
**WARNING:** Developers don't change light bulbs. It's a hardware problem.

**[back to top](#table-of-contents)** | **[back to section top](#1-document-conventions)**

## 2. Get the source code
Make sure `git` is installed:
```
sudo apt install git
```

Change to your `home` directory:
```
cd $HOME
```

Clone Kodi's current master branch:
```
git clone https://github.com/xbmc/xbmc kodi
```

**[back to top](#table-of-contents)**

## 3. Install the required packages
You can install the required packages using one of two methods: automagically or manually. Please use the former whenever possible.

**WARNING:** Oldest supported Ubuntu version is 16.04 (Xenial). It is possible to build on older Ubuntu releases but due to outdated packages it will require considerable fiddling. Sorry, you're on your own if you decide to go down that particular rabbit hole.

### 3.1. Get build dependencies automagically
Add the required PPAs (*nightly* and *build-depends*):
```
sudo add-apt-repository -s ppa:team-xbmc/xbmc-nightly
sudo add-apt-repository ppa:team-xbmc/xbmc-ppa-build-depends
sudo apt update
```

Super-duper magic command to get the build dependencies:
```
sudo apt build-dep kodi
```

**WARNING:** Do not use `aptitude` for the `build-dep` command. It doesn't resolve everything properly.

If at a later point you decide you do not want Kodi's PPAs on your system, removing them is as easy as:
```
sudo add-apt-repository -r ppa:team-xbmc/xbmc-nightly
sudo add-apt-repository -r ppa:team-xbmc/xbmc-ppa-build-depends
```

**NOTE:** For developers and anyone else who builds frequently it is recommended to install `ccache` to expedite subsequent builds of Kodi.

You can install it with:
```
sudo apt install ccache
```

**TIP:** If you have multiple computers at home, `distcc` will distribute build workloads of C and C++ code across several machines on a network. Team Kodi may not be willing to give support if problems arise using such a build configuration.

You can install it with:
```
sudo apt install distcc
```

### 3.2. Get build dependencies manually
If you get a `package not found` type of message with the below command, remove the offending package(s) from the install list and reissue the command. Take a note of the missing dependencies and, after a successful step completion, **[build the missing dependencies manually](#33-build-missing-dependencies)**.

Install build dependencies manually:
```
sudo apt install debhelper autoconf automake autopoint gettext autotools-dev cmake curl default-jre doxygen gawk gcc gdc gperf libasound2-dev libass-dev libavahi-client-dev libavahi-common-dev libbluetooth-dev libbluray-dev libbz2-dev libcdio-dev libcec4-dev libp8-platform-dev libcrossguid-dev libcurl4-openssl-dev libcwiid-dev libdbus-1-dev libegl1-mesa-dev libenca-dev libflac-dev libfontconfig-dev libfmt3-dev libfreetype6-dev libfribidi-dev libgcrypt-dev libgif-dev libgles2-mesa-dev libgl1-mesa-dev libglu1-mesa-dev libgnutls28-dev libgpg-error-dev libiso9660-dev libjpeg-dev liblcms2-dev liblircclient-dev libltdl-dev liblzo2-dev libmicrohttpd-dev libmysqlclient-dev libnfs-dev libogg-dev libpcre3-dev libplist-dev libpng-dev libpulse-dev libshairplay-dev libsmbclient-dev libsqlite3-dev libssh-dev libssl-dev libtag1-dev libtiff5-dev libtinyxml-dev libtool libudev-dev libva-dev libvdpau-dev libvorbis-dev libxkbcommon-dev libxmu-dev libxrandr-dev libxslt1-dev libxt-dev lsb-release python-dev python-pil python-imaging rapidjson-dev swig unzip uuid-dev yasm zip zlib1g-dev
```
**WARNING:** Make sure you copy paste the entire line or you might receive an error or miss a few dependencies.

Building for Wayland requires some extra packages:
```
sudo apt install libwayland-dev waylandpp-dev wayland-protocols
```

**WARNING:** Ubuntu 16.04 repositories don't have install candidates for `waylandpp-dev`and `wayland-protocols`. See **[build missing dependencies manually](#33-build-missing-dependencies)** section before you proceed.

Similarly, building for GBM also requires an extra package:
```
sudo apt install libgbm-dev
```
Optional packages that you might want to install for extra functionality (generating doxygen documentation, for instance):
```
sudo apt install doxygen libcap-dev libsndio-dev libmariadbd-dev
```

### 3.3. Build missing dependencies
See the general **[Linux build guide](README.Linux.md)** for reference.

Build and install missing dependencies from repositories (*wayland-protocols* and *waylandpp*). For **Ubuntu 16.04 only** :
```
cd $HOME/kodi
sudo make -C tools/depends/target/wayland-protocols PREFIX=/usr/local
sudo make -C tools/depends/target/waylandpp PREFIX=/usr/local
```

**[back to top](#table-of-contents)**

## 4. Build Kodi
See the general **[Linux build guide](README.Linux.md)** for reference.

**[back to top](#table-of-contents)**
