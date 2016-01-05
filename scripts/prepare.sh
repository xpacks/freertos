#! /bin/bash
set -euo pipefail
IFS=$'\n\t'

# Project home page:
# 	http://www.freertos.org

# Archives can be downloaded from:
# 	https://sourceforge.net/projects/freertos/files/FreeRTOS/

# RELEASE_VERSION="2.4.2"
RELEASE_VERSION="2.6.1"
RELEASE_ID="V${RELEASE_VERSION}"
# FOLDER_PREFIX=""
FOLDER_PREFIX="FreeRTOS "
NAME_PREFIX="FreeRTOS"
RELEASE_NAME="${NAME_PREFIX}v${RELEASE_VERSION}"
# RELEASE_NAME="${NAME_PREFIX}V${RELEASE_VERSION}"
ARCHIVE_NAME="${RELEASE_NAME}.zip"
ARCHIVE_URL="https://sourceforge.net/projects/freertos/files/FreeRTOS/${FOLDER_PREFIX}${RELEASE_ID}/${ARCHIVE_NAME}"

LOCAL_ARCHIVE_FILE="/tmp/xpacks/${ARCHIVE_NAME}"

for f in *
do
  if [ "${f}" == "scripts" ]
  then
    :
  else
    rm -rf "${f}"
  fi
done

if [ ! -f "${LOCAL_ARCHIVE_FILE}" ]
then
  mkdir -p $(dirname ${LOCAL_ARCHIVE_FILE})
  echo "${ARCHIVE_URL}"
  curl -o "${LOCAL_ARCHIVE_FILE}" -L "${ARCHIVE_URL}"
  if [ -z $(file "${LOCAL_ARCHIVE_FILE}" | grep 'Zip') ]
  then
    ls -l "${LOCAL_ARCHIVE_FILE}"
    file "${LOCAL_ARCHIVE_FILE}"
    exit 1
  fi
fi

unzip -q "${LOCAL_ARCHIVE_FILE}"

# For just in case...
find . -name '*.exe' -exec rm -v \{} \;

# FOLDER=""
FOLDER="FreeRTOS/"

# mv FreeRTOSV*/FreeRTOS .
# rm -rf FreeRTOSV* 

rm -rf \
${FOLDER}Demo \
${FOLDER}TraceCon \
${FOLDER}Source/portable/[BCIKoPRSTW]* \
${FOLDER}Source/portable/M[PS]* \
${FOLDER}Source/portable/GCC/[B-Z]* \
${FOLDER}Source/portable/GCC/ARM7* \

cp "${FOLDER}License/license.txt" "LICENSE.txt"

cat <<EOF >README.md
# FreeRTOS - Cross Platform Real Time Operating System 

This project, hosted on [GitHub](https://github.com/xpacks),
includes a selection of the FreeRTOS files.

## Version

* ${RELEASE_ID}

## Documentation

The FreeRTOS reference site is
http://www.freertos.org/a00106.html.

## Original files

The original files are stored in the \`originals\` branch.

These files were extracted from \`${ARCHIVE_NAME}\`.

To save space, only the FreeRTOS folder was preserved and the following folders/files were removed:

* Demo 
* TraceCon
* Source/portable/[BCIKoPRSTW]* 
* Source/portable/M[PS]* 
* Source/portable/GCC/[B-Z]* 
* Source/portable/GCC/ARM7* 

EOF

echo
echo Check if ok and when ready, issue: \`git commit -m ${ARCHIVE_NAME}\`