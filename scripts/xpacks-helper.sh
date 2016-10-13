#!/bin/bash
#set -euo pipefail
#IFS=$'\n\t'

# -----------------------------------------------------------------------------
# Bash helper script used in project generate.sh scripts.
# -----------------------------------------------------------------------------

# $1 $2 ... = port names, like ARM_CM3
do_add_freertos_xpack() {
  local pack_name='freertos'
  do_tell_xpack "${pack_name}-xpack"

  do_select_pack_folder "ilg/${pack_name}.git"

  do_prepare_dest "${pack_name}/FreeRTOS/include"
  do_add_content "${pack_folder}/FreeRTOS/Source/include"/* 

  do_prepare_dest "${pack_name}/FreeRTOS/src"
  do_add_content "${pack_folder}/FreeRTOS/Source"/*.c
  do_add_content "${pack_folder}/FreeRTOS/Source/portable/MemMang"

  do_prepare_dest "${pack_name}"
  do_add_content "${pack_folder}/cmsis-plus"

  while [ $# -ge 1 ]
  do
    do_prepare_dest "${pack_name}"
    do_add_content "${pack_folder}/FreeRTOS/Source/portable/GCC/$1"

    shift
  done
}

# -----------------------------------------------------------------------------
