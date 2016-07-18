# Set mirror
# TODO: move mirror logic to Macros.cmake
if(NOT DEFINED KODI_MIRROR)
  set(KODI_MIRROR "http://mirrors.kodi.tv")
endif()

# if(CRIS)

### Download and prepare Kodi's build deps

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/depsunpacked")

# Make sure we start with a fresh build deps folder
# TODO: only needed if we're not copying files to ${CORE_SOURCE_DIR}, i.e.,
# building out of tree. Lets see if I can make it work...
if(EXISTS "${CMAKE_BINARY_DIR}/depsready")
  message(STATUS "Dependency: Found '${CMAKE_BINARY_DIR}/depsready' folder. Erasing for good measure.")
  file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/depsready")
endif()
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/depsready")

# Parse dependency list
file(STRINGS ${CORE_SOURCE_DIR}/project/BuildDependencies/scripts/0_package.list PACKAGES)
foreach(LINE ${PACKAGES})
  if(NOT LINE MATCHES "^(\;.*)")
    list(APPEND DEPS_PACKAGES ${LINE})
  endif()
endforeach()
unset(PACKAGES)

# Download, extract and copy deps files
foreach(FILE ${DEPS_PACKAGES})
  # Get files
  if(EXISTS "${CMAKE_BINARY_DIR}/depscache/${FILE}")
    message(STATUS "Dependency: Found '${FILE}' in download cache. Skipping download.")
  else()
    message(STATUS "Dependency: Downloading '${FILE}' from mirrors.")
    if(VERBOSE)
      file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/${FILE}" "${CMAKE_BINARY_DIR}/depscache/${FILE}"
           SHOW_PROGRESS
           STATUS DOWNLOAD_STATUS)
    else()
    file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/${FILE}" "${CMAKE_BINARY_DIR}/depscache/${FILE}"
         STATUS DOWNLOAD_STATUS)
    endif()

    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS)
    if(DOWNLOAD_STATUS STREQUAL 0)
      message(STATUS "Dependency: Successfully downloaded '${FILE}'.")
    else()
      message(FATAL_ERROR "Dependency: Failed to download '${FILE}'.")
    endif()
  endif()

  # Decompress files
  message(STATUS "Dependency: Decompressing '${FILE}'.")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf "${CMAKE_BINARY_DIR}/depscache/${FILE}"
                          RESULT_VARIABLE STATUS_CODE
                          WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/depsunpacked")
  if(STATUS_CODE STREQUAL 0)
    message(STATUS "Dependency: Successfully decompressed '${FILE}'.")
  else()
    message(FATAL_ERROR "Dependency: Failed to decompress '${FILE}'.")
  endif()

  # Copy deps files to where they need to be
  # get_filename_component(... ... NAME_WE) doesn't work because our files have a
  # lot of dots in their names and it craps out invalid package names
  # strip extension instead
  string(REGEX REPLACE "\\.[^.]*$" "" FILE ${FILE})

  # Delete all files on package root folder. Apparently they aren't needed
  file(GLOB FILES_TO_DELETE "${CMAKE_BINARY_DIR}/depsunpacked/${FILE}/*")
  foreach(file ${FILES_TO_DELETE})
    file(REMOVE ${file})
  endforeach()
  unset(FILES_TO_DELETE)

  message(STATUS "Dependency: Copying files from '${CMAKE_BINARY_DIR}/depsunpacked/${FILE}/' to '${CMAKE_BINARY_DIR}/depsready/'.")
  # TODO: change to copy files to ${CORE_SOURCE_DIR} after testing
  # file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${FILE}/" DESTINATION "${CMAKE_BINARY_DIR}/depsready")
  file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${FILE}/" DESTINATION "${CORE_SOURCE_DIR}")
endforeach()
unset(DEPS_PACKAGES)


### Download MSYS2
# Check arch and process accordingly
if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86)
  set(MSYS2_FILE msys2-base-i686-20160205.tar.xz)
  set(MSYS2_FOLDER msys32)
else()
  set(MSYS2_FILE msys2-base-x86_64-20160205.tar.xz)
  set(MSYS2_FOLDER msys64)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/depscache/${MSYS2_FILE}")
  message(STATUS "Dependency: Found '${MSYS2_FILE}' in download cache. Skipping download.")
else()
  message(STATUS "Dependency: Downloading '${MSYS2_FILE}' from mirrors.")
  if(VERBOSE)
    file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/msys2/${MSYS2_FILE}"
                  "${CMAKE_BINARY_DIR}/depscache/${MSYS2_FILE}"
         SHOW_PROGRESS
         STATUS DOWNLOAD_STATUS)
  else()
    file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/msys2/${MSYS2_FILE}"
                  "${CMAKE_BINARY_DIR}/depscache/${MSYS2_FILE}"
         STATUS DOWNLOAD_STATUS)
  endif()

  list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS)
  if(DOWNLOAD_STATUS STREQUAL 0)
    message(STATUS "Dependency: Successfully downloaded '${MSYS2_FILE}'.")
  else()
    message(FATAL_ERROR "Dependency: Failed to download '${MSYS2_FILE}'.")
  endif()
endif()

# Decompress MSYS2
message(STATUS "Dependency: Decompressing '${MSYS2_FILE}'.")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf "${CMAKE_BINARY_DIR}/depscache/${MSYS2_FILE}"
                        RESULT_VARIABLE STATUS_CODE
                        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/depsunpacked")
if(STATUS_CODE STREQUAL 0)
  message(STATUS "Dependency: Successfully decompressed '${MSYS2_FILE}'.")
else()
  message(FATAL_ERROR "Dependency: Failed to decompress '${MSYS2_FILE}'.")
endif()

# Copy MSYS2 files to where they need to be
message(STATUS "Dependency: Copying files from '${CMAKE_BINARY_DIR}/depsunpacked/${MSYS2_FOLDER}/' to '${CORE_SOURCE_DIR}/project/BuildDependencies/${MSYS2_FOLDER}/'.")
# file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${MSYS2_FOLDER}/" DESTINATION "${CMAKE_BINARY_DIR}/depsready/${MSYS2_FOLDER}")
file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${MSYS2_FOLDER}/" DESTINATION "${CORE_SOURCE_DIR}/project/BuildDependencies/${MSYS2_FOLDER}")
unset(MSYS2_FILE)
unset(MSYS2_FOLDER)


### Download GNUtils
set(GNUTLS_FILE i686-gnutls-3.4.9-static.tar.gz)
set(GNUTLS_FOLDER local32)

if(EXISTS "${CMAKE_BINARY_DIR}/depscache/${GNUTLS_FILE}")
  message(STATUS "Dependency: Found '${GNUTLS_FILE}' in download cache. Skipping download.")
else()
  message(STATUS "Dependency: Downloading '${GNUTLS_FILE}' from mirrors.")
  if(VERBOSE)
    file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/msys2/locals/${GNUTLS_FILE}"
                  "${CMAKE_BINARY_DIR}/depscache/${GNUTLS_FILE}"
         SHOW_PROGRESS
         STATUS DOWNLOAD_STATUS)
  else()
    file(DOWNLOAD "${KODI_MIRROR}/build-deps/win32/msys2/locals/${GNUTLS_FILE}"
                  "${CMAKE_BINARY_DIR}/depscache/${GNUTLS_FILE}"
         STATUS DOWNLOAD_STATUS)
  endif()

  list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS)
  if(DOWNLOAD_STATUS STREQUAL 0)
    message(STATUS "Dependency: Successfully downloaded '${GNUTLS_FILE}'.")
  else()
    message(FATAL_ERROR "Dependency: Failed to download '${GNUTLS_FILE}'.")
  endif()
endif()

# Decompress GNUtils
message(STATUS "Dependency: Decompressing '${GNUTLS_FILE}'.")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf "${CMAKE_BINARY_DIR}/depscache/${GNUTLS_FILE}"
                        RESULT_VARIABLE STATUS_CODE
                        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/depsunpacked")
if(STATUS_CODE STREQUAL 0)
  message(STATUS "Dependency: Successfully decompressed '${GNUTLS_FILE}'.")
else()
  message(FATAL_ERROR "Dependency: Failed to decompress '${GNUTLS_FILE}'.")
endif()

# Copy GNUtils files to where they need to be
message(STATUS "Dependency: Copying files from '${CMAKE_BINARY_DIR}/depsunpacked/${GNUTLS_FOLDER}/' to '${CORE_SOURCE_DIR}/project/BuildDependencies/${GNUTLS_FOLDER}/'.")
# file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${MSYS2_FOLDER}/" DESTINATION "${CMAKE_BINARY_DIR}/depsready/${MSYS2_FOLDER}")
file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${GNUTLS_FOLDER}/" DESTINATION "${CORE_SOURCE_DIR}/project/BuildDependencies/${GNUTLS_FOLDER}")
unset(GNUTLS_FILE)
unset(GNUTLS_FOLDER)
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/depsunpacked")

# endif() #cris

###############################################################################
######################### CODE TESTED TO THIS POINT ###########################
###############################################################################

# Configure mintty
# TODO: Change user to $ENV{USERNAME} (windows username env variable)
# if(NOT EXISTS "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/home/$ENV{USERNAME}/.minttyrc")
#  file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/home/$ENV{USERNAME}/.minttyrc" "\
if(NOT EXISTS "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/home/h.udo/.minttyrc")
  file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/home/h.udo/.minttyrc" "\
BoldAsFont=no\n\
BackgroundColour=0,43,54\n\
ForegroundColour=147,161,161\n\
FontHeight=9\n\
FontSmoothing=full\n\
AllowBlinking=yes\n\
Font=DejaVu Sans Mono\n\
Columns=120\n\
Rows=30\n\
Term=xterm-256color\n\
CursorType=block\n\
ClicksPlaceCursor=yes\n\
Black=0,43,54\n\
BoldBlack=101,123,131\n\
Red=220,50,47\n\
BoldRed=232,115,113\n\
Green=133,153,0\n\
BoldGreen=199,228,0\n\
Yellow=181,137,0\n\
BoldYellow=255,193,2\n\
Blue=38,139,210\n\
BoldBlue=99,173,227\n\
Magenta=108,113,196\n\
BoldMagenta=162,165,217\n\
Cyan=42,161,152\n\
BoldCyan=71,207,197\n\
White=147,161,161\n\
BoldWhite=253,246,227")
endif()

# TODO: Unify arch verify/setup with block in the beginning of the file
if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86)
  set(ARCH_REPO i686)
else()
  set(ARCH_REPO x86_64)
endif()

# Configure pacman mirrors
#file(WRITE "${CMAKE_BINARY_DIR}/mirrorlist.msys" "\
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/pacman.d/mirrorlist.msys" "\
Server = http://mirrors.kodi.tv/build-deps/win32/msys2/repos/msys2/${ARCH_REPO}\n\
Server = http://repo.msys2.org/msys/${ARCH_REPO}\n\
Server = http://downloads.sourceforge.net/project/msys2/REPOS/MSYS2/${ARCH_REPO}\n\
Server = http://www2.futureware.at/~nickoe/msys2-mirror/msys/${ARCH_REPO}/")

file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/pacman.d/mirrorlist.mingw32" "\
Server = http://mirrors.kodi.tv/build-deps/win32/msys2/repos/mingw32\n\
Server = http://repo.msys2.org/mingw/i686\n\
Server = http://downloads.sourceforge.net/project/msys2/REPOS/MINGW/i686\n\
Server = http://www2.futureware.at/~nickoe/msys2-mirror/i686/")

file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/pacman.d/mirrorlist.mingw64" "\
Server = http://mirrors.kodi.tv/build-deps/win32/msys2/repos/mingw64\n\
Server = http://repo.msys2.org/mingw/x86_64\n\
Server = http://downloads.sourceforge.net/project/msys2/REPOS/MINGW/x86_64\n\
Server = http://www2.futureware.at/~nickoe/msys2-mirror/x86_64/")

# TODO: for what?
set($ENV{cygdrive} "no")
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/fstab" "\
none / cygdrive binary,posix=0,noacl,user 0 0\n\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\build\\          /build\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\local32\\        /local32\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\local64\\        /local64\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\msys64\\mingw32\\ /mingw32\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\msys64\\mingw64\\ /mingw64\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\downloads2\\     /var/cache/pacman/pkg\n\
${CORE_SOURCE_DIR}\\project\\BuildDependencies\\..\\..\\          /xbmc")
set($ENV{cygdrive} "yes")

# TODO: Check arch
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/pac-base-new.pk" "\
autoconf\n\
automake\n\
libtool\n\
m4\n\
make\n\
gettext\n\
patch\n\
pkg-config\n\
wget\n\
p7zip\n\
coreutils")

# file(WRITE "${CMAKE_BINARY_DIR}/pacman.sh" "pacman --noconfirm -S $(cat /etc/pac-base-new.pk | sed -e 's#\\##')\nsleep 3\nexit")
# file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/pacman.sh" "pacman --noconfirm --needed -S $(cat /etc/pac-base-new.pk | sed -e 's#\\\\##')")
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/pacman.sh" "pacman --noconfirm --needed -S $(cat /etc/pac-base-new.pk | sed -e 's#\\\\##')\nsleep 3\nexit")
execute_process(COMMAND "cmd" " /C ${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/usr/bin/mintty.exe -d -i /msys2.ico /usr/bin/bash --login ${CORE_SOURCE_DIR}/project/BuildDependencies/pacman.sh")
file(REMOVE "${CORE_SOURCE_DIR}/project/BuildDependencies/pacman.sh")

# TODO: recheck. If I read the log right, this is only need if ${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/usr/ssl/cert.pem == 0
# file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/cert.sh" "update-ca-trust\nsleep 3\nexit")
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/cert.sh" "update-ca-trust")
execute_process(COMMAND "cmd" " /C ${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/usr/bin/mintty.exe -d -i /msys2.ico /usr/bin/bash --login ${CORE_SOURCE_DIR}/project/BuildDependencies/cert.sh")
file(REMOVE "${CORE_SOURCE_DIR}/project/BuildDependencies/cert.sh")

file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/etc/pac-mingw32-new.pk" "\
mingw-w64-i686-dlfcn\n\
mingw-w64-i686-gcc\n\
mingw-w64-i686-gcc-libs\n\
mingw-w64-i686-gettext\n\
mingw-w64-i686-libiconv\n\
mingw-w64-i686-libgpg-error\n\
mingw-w64-i686-libpng\n\
mingw-w64-i686-yasm\n\
mingw-w64-i686-nettle\n\
mingw-w64-i686-libtasn1\n\
mingw-w64-i686-openssl")

# file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/mingw32.sh" "pacman --noconfirm --needed -S $(cat /etc/pac-mingw32-new.pk | sed -e 's#\\\\##')\nsleep 3\nexit")
file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/mingw32.sh" "pacman --noconfirm --needed -S $(cat /etc/pac-mingw32-new.pk | sed -e 's#\\\\##')")
execute_process(COMMAND "cmd" " /C ${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/usr/bin/mintty.exe -d -i /msys2.ico /usr/bin/bash --login ${CORE_SOURCE_DIR}/project/BuildDependencies/mingw32.sh")
file(REMOVE "${CORE_SOURCE_DIR}/project/BuildDependencies/mingw32.sh")

if(EXISTS "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.dll.a")
  file(REMOVE "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.dll")
endif()

if(EXISTS "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.a")
  file(RENAME "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.a"
              "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/libxvidcore.a")
endif()

if(EXISTS "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.dll.a")
  file(RENAME "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.dll.a "
              "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/xvidcore.dll.a.dyn")
endif()

file(GLOB_RECURSE LIBRARIES "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/*.dll.a")
# file(GLOB LIBRARIES "${CORE_SOURCE_DIR}/project/BuildDependencies/msys64/mingw32/lib/*.dll.a")
foreach(file ${LIBRARIES})
  if(EXISTS "${file}.dyn")
    file(REMOVE "${file}.dyn")
  endif()
  file(RENAME "${file}" "${file}.dyn")
endforeach()

file(WRITE "${CORE_SOURCE_DIR}/project/BuildDependencies/local32/etc/profile.local" "\
#\n\
# /local32/etc/profile.local\n\
#\n\n\
MSYSTEM=MINGW32\n\n\
alias dir='ls -la --color=auto'\n\
alias ls='ls --color=auto'\n\
export CC=gcc\n\
export python=/usr/bin/python\n\n\
MSYS2_PATH=\"/usr/local/bin:/usr/bin\"\n\
MANPATH=\"/usr/share/man:/mingw32/share/man:/local32/man:/local32/share/man\"\n\
INFOPATH=\"/usr/local/info:/usr/share/info:/usr/info:/mingw32/share/info\"\n\
MINGW_PREFIX=\"/mingw32\"\n\
MINGW_CHOST=\"i686-w64-mingw32\"\n\
export MSYSTEM MINGW_PREFIX MINGW_CHOST\n\n\
DXSDK_DIR=\"/mingw32/i686-w64-mingw32\"\n\
ACLOCAL_PATH=\"/mingw32/share/aclocal:/usr/share/aclocal\"\n\
PKG_CONFIG_LOCAL_PATH=\"/local32/lib/pkgconfig\"\n\
PKG_CONFIG_PATH=\"/local32/lib/pkgconfig:/mingw32/lib/pkgconfig\"\n\
CPPFLAGS=\"-I/local32/include -D_FORTIFY_SOURCE=2\"\n\
CFLAGS=\"-I/local32/include -mms-bitfields -mthreads -mtune=generic -pipe\"\n\
CXXFLAGS=\"-I/local32/include -mms-bitfields -mthreads -mtune=generic -pipe\"\n\
LDFLAGS=\"-L/local32/lib -mthreads -pipe\"\n\
export DXSDK_DIR ACLOCAL_PATH PKG_CONFIG_PATH PKG_CONFIG_LOCAL_PATH CPPFLAGS CFLAGS CXXFLAGS LDFLAGS MSYSTEM\n\n\
PYTHONHOME=/usr\n\
PYTHONPATH=\"/usr/lib/python2.7:/usr/lib/python2.7/Tools/Scripts\"\n\n\
PATH=\".:/local32/bin:/mingw32/bin:$\{MSYS2_PATH\}:$\{INFOPATH\}:$\{PYTHONHOME\}:$\{PYTHONPATH\}:$\{PATH\}\"\n\
PS1='\\[\\033[32m\\]\\u@\\h \\[\\e[33m\\]\\w\\[\\e[0m\\]\\n\\$ '\n\
export PATH PS1\n\n\
# package build directory\n\
LOCALBUILDDIR=/build\n\
# package installation prefix\n\
LOCALDESTDIR=/local32\n\
export LOCALBUILDDIR LOCALDESTDIR\n\n\
BITS='32bit'\n\
export BITS")

# TODO: check needed folders
# mkdir C:\kodiws\kodi\project\BuildDependencies\local32\bin

### Step 3 - FFMpeg

file(STRINGS ${CORE_SOURCE_DIR}/tools/depends/target/ffmpeg/FFMPEG-VERSION VER)
string(REGEX MATCH "VERSION=[^ ]*$.*" FFMPEG_VER "${VER}")
list(GET FFMPEG_VER 0 FFMPEG_VER)
string(SUBSTRING "${FFMPEG_VER}" 8 -1 FFMPEG_VER)
string(REGEX MATCH "BASE_URL=([^ ]*)" FFMPEG_BASE_URL "${VER}")
list(GET FFMPEG_BASE_URL 0 FFMPEG_BASE_URL)
string(SUBSTRING "${FFMPEG_BASE_URL}" 9 -1 FFMPEG_BASE_URL)

${FFMPEG_BASE_URL}/${FFMPEG_VER}.tar.gz
file(DOWNLOAD "${FFMPEG_BASE_URL}/${FFMPEG_VER}.tar.gz" "${CMAKE_BINARY_DIR}/depscache/${FFMPEG_VER}.tar.gz"
     SHOW_PROGRESS
     STATUS DOWNLOAD_STATUS)
