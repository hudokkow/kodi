include(${CORE_SOURCE_DIR}/project/cmake/scripts/common/Macros.cmake)

# Get deps list
core_file_read_filtered(prebuilt_list ${PROJECT_SOURCE_DIR}/scripts/${CORE_SYSTEM_NAME}/PrebuiltDepsList.txt)
string(REPLACE " " ";" prebuilt_list "${prebuilt_list}")

# Check if user passed a download cache dir. Set it otherwise
if(NOT DOWNLOAD_CACHE_DIR)
  set(DOWNLOAD_CACHE_DIR ${CMAKE_BINARY_DIR}/downloads)
endif()

# Check if user passed a depends dir. Set it otherwise
if(NOT DOWNLOAD_DEPENDS_DIR)
  set(DOWNLOAD_DEPENDS_DIR ${CMAKE_BINARY_DIR}/depends)
endif()

set(index 0)
list(LENGTH prebuilt_list size)
while(index LESS size)
  # Get sha1sum
  list(GET prebuilt_list ${index} SHA1)

  # Increase by one to get the filename
  math(EXPR index "${index} + 1")
  list(GET prebuilt_list ${index} FILE)

  # Download and verify file integrity
  download_file(${APP_MIRRORS}/build-deps/win32/${FILE} ${SHA1} ${DOWNLOAD_CACHE_DIR})

  # Decompress file
  decompress_file(${DOWNLOAD_CACHE_DIR}/${FILE} ${DOWNLOAD_DEPENDS_DIR})

  # get_filename_component(... ... NAME_WE) doesn't work because our files have a
  # lot of dots in their names and it craps out invalid package names
  # strip extension instead
  string(REGEX REPLACE "\\.[^.]*$" "" FILE ${FILE})

  # Copy deps files to where they need to be
  message(STATUS "Dependency: Copying files from '${DOWNLOAD_DEPENDS_DIR}/${FILE}/' to '${DOWNLOAD_DEPENDS_DIR}'")
  file(COPY "${DOWNLOAD_DEPENDS_DIR}/${FILE}/" DESTINATION "${DOWNLOAD_DEPENDS_DIR}")
  file(REMOVE_RECURSE ${DOWNLOAD_DEPENDS_DIR}/${FILE}/)
  # TODO: change to copy files to ${CORE_SOURCE_DIR} after testing
  # file(COPY "${CMAKE_BINARY_DIR}/depsunpacked/${FILE}/" DESTINATION "${CORE_SOURCE_DIR}")

  # Increase by one to get the sha1 + filename combo of next item
  math(EXPR index "${index} + 1")
endwhile()

# Delete all files on depends root folder. Apparently they aren't needed
file(GLOB FILES_TO_DELETE "${DOWNLOAD_DEPENDS_DIR}/*")
foreach(file ${FILES_TO_DELETE})
  file(REMOVE ${file})
endforeach()
unset(FILES_TO_DELETE)


  # extract dep name from filename (match until the first '-')
  #string(REGEX REPLACE "-.*$" "" NAME ${FILE})

#    ExternalProject_Add(prebuilt_${NAME}
#                        ${CMAKE_BINARY_DIR}/prebuilt/prebuilt_${NAME}
#                        #--Download step--------------
#                        URL ${APP_MIRRORS}/build-deps/win32/${FILE}
#                        URL_HASH SHA1=${SHA1}
#                        #--Configure step-------------
#                        CONFIGURE_COMMAND ""
#                        #--Build step-----------------
#                        BUILD_COMMAND ""
#                        #--Install step---------------
#                        UPDATE_COMMAND "" # Skip annoying updates for every build
#                        INSTALL_COMMAND "")

