![Kodi Logo](resources/banner_slim.png)

# Debian/Ubuntu build guide

## Table of Contents
1. **[Document conventions](#1-document-conventions)**
2. **[Getting the source code](#2-getting-the-source-code)**
3. **[Installing the required packages](#3-installing-the-required-packages)**  
  3.1. **[Get build dependencies automagically](#31-get-build-dependencies-automagically)**  
  3.2. **[Get build dependencies manually](#32-get-build-dependencies-manually)**  
  3.3. **[Manually build missing dependencies](#33-manually-build-missing-dependencies)**
4. **[Build Kodi](#4-build-kodi)**  
  4.1. **[Build directory](#41-build-directory)**  
  4.2. **[Build configuration](#42-build-configuration)**  
  4.3. **[Build](#43-build)**
5. **[Test suite](#5-test-suite)**
6. **[Build binary add-ons](#6-build-binary-add-ons)**  
  6.1. **[Building all add-ons](#61-building-all-add-ons)**  
  6.2. **[Building specific add-ons](#62-building-specific-add-ons)**
7. **[Running Kodi](#7-running-kodi)**
8. **[Uninstalling](#8-uninstalling)**

### 1. Document conventions
Lines prefixed with `//` are comments that provide context.
Lines without prefix are commands that need to be run at the command line, one at a time and in the provided order.

```
// change to Kodi's directory
cd kodi
// clean source tree
git clean -xffd
// create build directory
mkdir kodi-build
// change to build directory
cd kodi-build
```

Several different strategies are used to draw your attention to certain pieces of information. In order of how critical the information is, these items are marked as a note, tip, or warning. For example: 
 
**NOTE:** Linux is user friendly... It's just very particular about who its friends are.  
**TIP:** Algorithm is what developers call code they do not want to explain.  
**WARNING:** Developers don't change light bulbs. It's a hardware problem.

**[back to top](#table-of-contents)**

### 2. Getting the source code
Make sure you have `git` installed:
```
sudo apt install git
```
Clone current master branch:
```
git clone https://github.com/xbmc/xbmc kodi
```
Clone a specific branch:
```
git clone -b <branch-name> https://github.com/xbmc/xbmc kodi-<branch-name>
```

**[back to top](#table-of-contents)**

### 3. Installing the required packages
Some packages for older Ubuntu versions might not be available in the repositories. If you get a `Could NOT find...` error message during CMake configuration step, take a note of the missing dependency(ies) and **[build the missing dependencies manually](#33-manually-build-missing-dependencies)**.


**WARNING:** Oldest supported Ubuntu version is 16.04 (Xenial). It is possible to build on older Ubuntu releases but due to outdated packages, no full C++11 support, etc., it will require considerable fiddling. Sorry, you're on your own if you decide to go down that particular rabbit hole.

You can install the required packages using one of two methods: automagically or manually. Please use the former whenever possible.

#### 3.1. Get build dependencies automagically
Add the required PPAs (*unstable* and *build-depends*):
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

For developers and anyone else who builds frequently it is recommended to install `ccache` to expedite subsequent builds of Kodi.
```
sudo apt install ccache
```

**TIP:** If you have multiple computers at home, `distcc` will distribute build workloads of C and C++ code across several machines on a network. Team Kodi may not give support if problems arise using such a build configuration. Nonetheless, you can install it with:
```
sudo apt install distcc
```

#### 3.2. Get build dependencies manually
If you get a `package not found` message with the below command, remove the offending package(s) from the install list and reissue the command. After the step completion, take a note of the missing dependency(ies) and **[build the missing dependencies manually](#33-manually-build-missing-dependencies)**.

Install build dependencies manually:
```
sudo apt install debhelper autoconf automake autopoint gettext autotools-dev cmake curl default-jre doxygen gawk gcc gdc gperf libasound2-dev libass-dev libavahi-client-dev libavahi-common-dev libbluetooth-dev libbluray-dev libbz2-dev libcdio-dev libcec4-dev libp8-platform-dev libcrossguid-dev libcurl4-gnutls-dev libcwiid-dev libdbus-1-dev libegl1-mesa-dev libenca-dev libflac-dev libfontconfig-dev libfmt3-dev libfreetype6-dev libfribidi-dev libgcrypt-dev libgif-dev libgl1-mesa-dev libglu1-mesa-dev libgnutls28-dev libgpg-error-dev libiso9660-dev libjpeg-dev liblcms2-dev libltdl-dev liblzo2-dev libmicrohttpd-dev libmysqlclient-dev libnfs-dev libogg-dev libpcre3-dev libplist-dev libpng-dev libpulse-dev libshairplay-dev libsmbclient-dev libsqlite3-dev libssh-dev libssl-dev libtag1-dev libtiff5-dev libtinyxml-dev libtool libudev-dev libva-dev libvdpau-dev libvorbis-dev libxkbcommon-dev libxmu-dev libxrandr-dev libxslt1-dev libxt-dev lsb-release python-dev python-pil python-imaging rapidjson-dev swig unzip uuid-dev yasm zip zlib1g-dev
```
**WARNING:** Make sure you copy paste the entire line or you might receive an error or miss a few dependencies.

If you want to build for Wayland instead of X11, you will also need:
```
sudo apt install waylandpp-dev wayland-protocols
```
Optional packages that you might want to install for extra functionality (generating doxygen documentation, for instance):
```
sudo apt install doxygen libcap-dev libsndio-dev libmariadb-client-lgpl-dev
```

#### 3.3. Manually build missing dependencies
Some packages are missing or are outdated in older Ubuntu versions. Notably `libcrossguid-dev, libfmt3-dev, waylandpp and wayland-protocols` are known to be outdated or missing. Fortunately we provide an easy way to build them.

```
// build and install crossguid
sudo make -C tools/depends/target/crossguid PREFIX=/usr/local
// build and install libfmt
sudo make -C tools/depends/target/libfmt PREFIX=/usr/local
// build and install wayland-protocols
sudo make -C tools/depends/target/wayland-protocols PREFIX=/usr/local
// build and install waylandpp
sudo make -C tools/depends/target/waylandpp PREFIX=/usr/local
```
**WARNING:** Building `waylandpp` has some dependencies of its own, namely `scons, libwayland-dev (>= 1.11.0) and libwayland-egl1-mesa`

**TIP:** Complete list of dependencies ready to build is available **[here](https://github.com/xbmc/xbmc/tree/master/tools/depends/target)**.

**[back to top](#table-of-contents)** | **[back to section top](#4-build-kodi)**

### 4. Build Kodi
#### 4.1. Create a build directory and change to it:
```
mkdir kodi-build
cd kodi-build
```

#### 4.2. Build configuration
For X11:
```
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```
For Wayland:
```
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCORE_PLATFORM_NAME=wayland -DWAYLAND_RENDER_SYSTEM=gl
```
**NOTE:** You can use `gles` instead of `gl` if you want to build with `GLES`.

#### 4.3. Build
```
cmake --build . -- VERBOSE=1
```
    
**TIP:** By adding `-j<number>` to the build command, you can choose how many concurrent jobs will be used. This expedites the build process.
For an *octacore* CPU the command should be:
```
cmake --build . -- VERBOSE=1 -j8
```

After the build process completes successfully you can test your shiny new Kodi build while in the build directory:
```
./kodi.bin
```

If everything was OK during your test you can now install the binaries to their place, in this example */usr/local*.
```
sudo make install
```
This will install Kodi in the prefix provided in **[section 4.2](#42-build-configuration)**.

**TIP:** To override Kodi's install location, use `DESTDIR=<path>`. For example:
```
make install DESTDIR=$HOME/kodi
```

**[back to top](#table-of-contents)** | **[back to section top](#4-build-kodi)**

### 5. Test suite
Kodi has a test suite which uses the Google C++ Testing Framework. This framework is provided directly in Kodi's source tree.

Build and run Kodi's test suite:
```
make check
```
Build Kodi's test suite without running it:
```
make kodi-test
```
Run Kodi's test suite manually:
```
./kodi-test
```
Show Kodi's test suite *help* notes:
```
./kodi-test --gtest_help
```

Useful options:
```
--gtest_list_tests
  List the names of all tests instead of running them.
  The name of TEST(Foo, Bar) is "Foo.Bar".

--gtest_filter=POSITIVE_PATTERNS[-NEGATIVE_PATTERNS]
  Run only the tests whose name matches one of the positive patterns but
  none of the negative patterns. '?' matches any single character; '*'
  matches any substring; ':' separates two patterns.
```

**[back to top](#table-of-contents)** | **[back to section top](#5-test-suite)**

### 6. Build binary add-ons
You can find a complete list of official available binary add-ons **[here](https://github.com/xbmc/repo-binary-addons)**.

#### 6.1. Building all add-ons
```
make -C tools/depends/target/binary-addons PREFIX=/usr/local
```

#### 6.2. Building specific add-ons
```
make -C tools/depends/target/binary-addons PREFIX=/usr/local ADDONS="audioencoder.flac pvr.vdr.vnsi audiodecoder.snesapu"
```
**NOTE:** `PREFIX=/usr/local` should match Kodi's prefix used in **[section 4.2](#42-build-configuration)**.

**[back to top](#table-of-contents)**

### 7. Running Kodi
If you chose to install Kodi using */usr* or */usr/local* as the `PREFIX=`, you can just issue *kodi* in a terminal session.

If you have overridden `PREFIX=` to install Kodi into some non-standard location, you will have to run Kodi directly:
```
$PREFIX/bin/kodi
```
To run Kodi in *portable* mode (useful for testing):
```
$PREFIX/bin/kodi -p
```

### 8. Uninstalling
```
sudo make uninstall
```
**WARNING:**: If you reran CMakes' configure step with a different `PREFIX=`, you will need to rerun configure with the correct `PREFIX=` for this step to work correctly.

If you would like to also remove any settings and third-party addons (skins, scripts, etc.) and Kodi configuration files, you should also run:
```
rm -rf ~/.kodi
```

**[back to top](#table-of-contents)**
