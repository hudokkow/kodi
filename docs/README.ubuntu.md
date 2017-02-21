![Kodi logo](https://raw.githubusercontent.com/xbmc/xbmc-forum/master/xbmc/images/logo-sbs-black.png)

# Kodi for Ubuntu and Debian based systems

1. [Introduction](#introduction)
2. [Getting the source code](#2-getting-the-source-code)
3. [Installing the required Ubuntu packages](#3-installing-the-required-Ubuntu-packages)  
  3.1 [Single command to get all build dependencies](#31-single-command-to-get-all-build-dependencies)  
  3.2 [Manual dependency installation](#32-manual-dependency-installation)  
4. [How to compile](#4-how-to-compile)
5. [Uninstalling](#5-uninstalling)

## 1. Introduction

A graphics-adapter with OpenGL acceleration is highly recommended. 24/32 bitdepth is required along with OpenGL.

## 2. Getting the source code

```console
cd $HOME
git clone git://github.com/xbmc/xbmc.git kodi
```

**Note:** You can clone any specific branch

```console
git clone -b <branch> git://github.com/xbmc/xbmc.git kodi
```

## 3. Installing the required Ubuntu packages

Two methods exist to install the required Ubuntu packages:

**Note:** For supported older Ubuntu versions, some packages might be outdated. You can either compile them manually or use our backports available from our official stable PPA:

```console
http://launchpad.net/~team-xbmc/+archive/ppa
```
### 3.1 Single command to get all build dependencies

**Note:** Only supported on Ubuntu >= 11.10 (oneiric)

You can get all build dependencies used for building the packages on the PPA

Add the unstable PPA:

For <= 12.04 LTS:
```console
sudo apt-get install python-software-properties
sudo add-apt-repository ppa:team-xbmc/xbmc-nightly
```

For >= 14.04 LTS:
```console
sudo apt-get install software-properties-common
sudo add-apt-repository -s ppa:team-xbmc/xbmc-nightly
```

Add build-depends PPA:
```console
sudo add-apt-repository ppa:team-xbmc/xbmc-ppa-build-depends
```

After adding the PPA, install build deps with
```console
sudo apt-get update
sudo apt-get build-dep kodi
```

**Optional:** If you do not want Kodi to be installed via PPA, you can removed the PPAs with
```console
sudo add-apt-repository -r ppa:team-xbmc/xbmc-nightly
sudo add-apt-repository -r ppa:team-xbmc/xbmc-ppa-build-depends
```

**Note:** Do not use "aptitude" for the build-dep command. It doesn't resolve everything properly.

For developers and anyone else who compiles frequently, it is recommended to use ccache
```console
sudo apt-get install ccache
```

**Tip:** For those with multiple computers at home, you can try `distcc`. We do not provide support for its usage.
```console
sudo apt-get install distcc
```

### 3.2 Manual dependency installation

Ubuntu >= 7.04:
```console
sudo apt-get install automake bison build-essential cmake curl cvs default-jre fp-compiler gawk gdc gettext git-core gperf libasound2-dev libass-dev libbz2-dev libcap-dev libcdio-dev libcurl3 libcurl4-openssl-dev libdbus-1-dev libfontconfig-dev libegl1-mesa-dev libfreetype6-dev libfribidi-dev libgif-dev libiso9660-dev libjpeg-dev liblzo2-dev libmicrohttpd-dev libmysqlclient-dev libnfs-dev libpcre3-dev libplist-dev libpng-dev libpulse-dev libsdl2-dev libsmbclient-dev libsqlite3-dev libssh-dev libssl-dev libtinyxml-dev libtool libudev-dev libusb-dev libva-dev libvdpau-dev libxml2-dev libxmu-dev libxrandr-dev libxrender-dev libxslt1-dev libxt-dev libyajl-dev mesa-utils nasm pmount python-dev python-imaging python-sqlite swig unzip uuid-dev yasm zip zlib1g-dev
```

Ubuntu >= 10.10:
```console
sudo apt-get install autopoint libltdl-dev
```

Ubuntu >= 12.04 LTS (backport for Precise of libyajl2):
```console
sudo add-apt-repository ppa:team-xbmc/xbmc-nightly
sudo apt-get install libyajl-dev
```

**Note:** Ubuntu Precise users also need an upgraded GCC, else compile will fail.

Ubuntu >= 12.10:
```console
sudo apt-get install libtag1-dev
```

On 8.10 and older versions, libcurl is outdated and thus Kodi will not compile properly. In this case you will have to manually compile the latest version.
```console
wget http://curl.sourceforge.net/download/curl-7.19.7.tar.gz
tar -xzf curl-7.19.7.tar.gz
cd curl-7.19.7
./configure --disable-ipv6 --without-libidn --disable-ldap --prefix=/usr
make
sudo make install
```

Ubuntu <= 12.04
Kodi needs a new version of taglib other than what is available at this time. Use pre-packaged from the Kodi build-depends PPA.
```console
sudo apt-get install libtag1-dev
```

or build and install manually into `usr/local`
```console
sudo apt-get remove libtag1-dev
make -C lib/taglib
sudo make -C lib/taglib install
```

**NOTICE:** crossguid / libcrossguid-dev all Linux distributions.
Kodi now requires crossguid which is not available in Ubuntu repositories at this time.
If build-deps PPA doesn't provide a pre-packaged version for your distribution, you need to build it manually (see below).

Use prepackaged from the Kodi build-depends PPA.
```console
sudo apt-get install libcrossguid-dev
```

or build and install manually into `usr/local`
```console
make -C tools/depends/target/crossguid PREFIX=/usr/local
```

Unless you are proficient with how Linux libraries and versions work, do not try to provide it yourself, as you will likely mess up for other programs.

## 4. How to compile

```console
cmake <KODI_SRC>
cmake --build . -- VERBOSE=1 -j$(nproc)
```
or
```console
make VERBOSE=1 -j$(nproc)
```

After build finishes, start kodi with
```console
./kodi.bin
```


4.1. Test Suite

See README.linux


## 5. Uninstalling

Remove any PPA installed Kodi:
```console
sudo apt-get remove kodi* xbmc*
```

See README.linux/Uninstalling for removing compiled versions of Kodi.
