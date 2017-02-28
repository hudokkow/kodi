# tips, hats and bling
# https://github.com/tomahawk-player/tomahawk/blob/master/CMakeModules/NSIS.template.in
# https://github.com/tomahawk-player/tomahawk/commit/18aba6856b5aec4d60627739b96897ba30e989f7


# include Macros.cmake to automate generation of time/date stamps, maintainer, etc.
include(${CMAKE_SOURCE_DIR}/cmake/scripts/common/Macros.cmake)

# App name
set(CPACK_PACKAGE_NAME ${APP_NAME})

# Vendor
set(CPACK_PACKAGE_VENDOR ${COMPANY_NAME})

# Package short description
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Kodi - Home Theater Software")

# Generated installer filename
string(TIMESTAMP PACKAGE_TIMESTAMP "%Y%m%d.%H%M" UTC)
core_find_git_rev(RELEASE_IDENTIFIER)
core_find_git_branch(GIT_BRANCH)

set(PACKAGE_NAME_VERSION ${APP_NAME}Setup-git${PACKAGE_TIMESTAMP}-${RELEASE_IDENTIFIER}-${GIT_BRANCH})
unset(PACKAGE_TIMESTAMP)
unset(RELEASE_IDENTIFIER)
unset(GIT_BRANCH)

# App install dir
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APP_NAME})

# Root install dir
# TODO: change to "$PROGRAMFILES64" after switch
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")

# Installer header image
# If problems arise: http://stackoverflow.com/questions/28768417/how-to-set-an-icon-in-nsis-install-cmake
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/tools/windows/packaging/media/installer/header.bmp")

# Installer/uninstaller icon
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/tools/windows/packaging/media/application.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/tools/windows/packaging/media/application.ico")

# Left welcome image
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/tools/windows/packaging/media/installer/welcome-left.bmp")
set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/tools/windows/packaging/media/installer/welcome-left.bmp")

# sidebar image
# http://public.kitware.com/pipermail/cmake/2011-April/044017.html

# License file
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.GPL")

# Start menu entry
set(CPACK_PACKAGE_EXECUTABLES "kodi" "Kodi")

