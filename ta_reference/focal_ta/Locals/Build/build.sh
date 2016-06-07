#!/bin/bash

t_sdk_root=$(readlink -f $(dirname $(readlink -f $0))/../../..)

source ${t_sdk_root}/setup.sh

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
mkdir -p Out/Public
cp -f \
	Locals/Code/public/* \
	Out/Public/
echo "Running make..."	
make -f Locals/Code/makefile.mk "$@"


#cp $(DR_DIR)/Debug/obj-local/Locals/Code/focal_lib_test.o $(DR_DIR)/Debug/obj-local/Locals/Code/focal_lib_test.ol
#########add by wenfs
#cp Debug/obj-local/Locals/Code/focal_lib_test.o Debug/obj-local/Locals/Code/focal_lib_test.ol

#/home/tee/trustonic/mtk/alps/prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-4_8-2014q3/bin/arm-none-eabi-ar -rcs Debug/obj-local/Locals/Code/focal_lib_test.lib Debug/obj-local/Locals/Code/focal_lib_test.ol

##########add by wenfs

