#!/bin/bash

t_sdk_root=$(readlink -f $(dirname $(readlink -f $0))/../../..)
source ${t_sdk_root}/setup.sh


cd $(dirname $(readlink -f $0))
cd ../..

#DRV_NAME=drfocal
if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=Debug
  OPTIM=debug
fi

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export DRSDK_DIR_SRC=${COMP_PATH_DrSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
export DRSDK_DIR=${COMP_PATH_DrSdk}
mkdir -p Out/Public
cp -f \
  Locals/Code/public/* \
  Out/Public/
echo "Running make..."	
make -f Locals/Code/makefile.mk "$@"

if [ $? -ne 0 ]; then
	echo "[ERROR] <t-driver build failed!"
	exit 1;
fi
