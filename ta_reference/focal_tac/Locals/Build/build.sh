#!/bin/bash

t_sdk_root=$(readlink -f $(dirname $(readlink -f $0))/../../..)

if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=Out/Bin/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=Out/Bin/Debug
  OPTIM=debug
fi

#OUT_LIBS=libs

source ${t_sdk_root}/setup.sh

# go to project root
cd $(dirname $(readlink -f $0))/../..

### ---------------- Generic Build Command ----------------

# run NDK build
${NDK_BUILD} \
    -B \
    NDK_DEBUG=1 \
    NDK_PROJECT_PATH=Locals/Code \
    NDK_APPLICATION_MK=Locals/Code/Application.mk \
    NDK_MODULE_PATH=${t_sdk_root} \
    NDK_APP_OUT=Out/_build \
    APP_BUILD_SCRIPT=Locals/Code/Android.mk \
    APP_OPTIM=$OPTIM

mkdir -p $OUT_DIR
#mkdir -p $OUT_LIBS

cp -r $PWD/Out/_build/local/$TAC_APP_ABI $OUT_DIR
#cp -r $PWD/Locals/Code/libs/* $OUT_LIBS/

echo
echo Output directory of build is $PWD/$OUT_DIR

