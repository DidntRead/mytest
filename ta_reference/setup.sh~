#!/bin/bash
#
# This file sets the path variables and resolves the dependencies for the
# different Makefiles included in the release.
#
# If you prefer to use the ARM DS-5 tools instead of GNU, please set
# 1) TOOLCHAIN=ARM
# 2) ARM_RVCT_PATH and LM_LICENSE_FILE according to your environment.
#
################### EDIT HERE ####################

export COMP_PATH_ROOT=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

# TOOLCHAIN : ARM, GNU
# If variable is not set, use GNU by default
export TOOLCHAIN=${TOOLCHAIN:-"GNU"}

# Mode used for building samples
export MODE=${MODE:-"Release"}

if [ $TOOLCHAIN == GNU ]; then
    # GCC Compiler variables
      export CROSS_GCC_PATH=/opt/gcc-arm-none-eabi-4_8-2014q3
      export CROSS_GCC_PATH_INC=${CROSS_GCC_PATH}/arm-none-eabi/include
      export CROSS_GCC_PATH_LIB=${CROSS_GCC_PATH}/arm-none-eabi/lib
      export CROSS_GCC_PATH_LGCC=${CROSS_GCC_PATH}/lib/gcc/arm-none-eabi/4.8.4
      export CROSS_GCC_PATH_BIN=${CROSS_GCC_PATH}/bin
fi

if [ $TOOLCHAIN == ARM ]; then
    export ARM_RVCT_PATH=/usr/local/DS-5
  #  export LM_LICENSE_FILE= # DS-5 license     - e.g.: /home/user/DS5PE-*.dat
    export ARM_RVCT_PATH_BIN=$ARM_RVCT_PATH/bin
    export ARM_RVCT_PATH_INC=$ARM_RVCT_PATH/include
    export ARM_RVCT_PATH_LIB=$ARM_RVCT_PATH/lib
fi

export ANDROID_NDK_LOCATION=/opt/android-ndk-r10c
#export ANDROID_HOME=$ANDROID_SDK_LOCATION

#export APP_ABI=${APP_ABI:-"armeabi-v7a"}
export APP_ABI=${APP_ABI:-"arm64-v8a"}

# Android NDK Directory
export NDK_BUILD=/opt/android-ndk-r10c/ndk-build

# Android SDK Directory
export ANDROID_HOME=/home/samuel/bin/sdk

# Java Home Directory
export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-amd64

# Ant application for building TSdkSample
#export ANT_PATH=/usr/bin/
#export PATH=${ANT_PATH}:$PATH

######################################################
# Checks
######################################################

if [ $TOOLCHAIN == GNU ]; then
    if [[ -z "$CROSS_GCC_PATH" ]] ;then
        echo "CROSS_GCC_PATH is not set in setup.sh"
        exit 1
    fi
fi

if [[ -z "$NDK_BUILD" ]] ;then
    echo "NDK_BUILD is not set in setup.sh"
    exit 1
fi

if [[ -z "$ANDROID_HOME" ]] ;then
    echo "ANDROID_HOME is not set in setup.sh"
    exit 1
fi

if [[ -z "$JAVA_HOME" ]] ;then
    echo "JAVA_HOME is not set in setup.sh"
    exit 1
fi

#if [[ -z "$ANT_PATH" ]] ;then
#    echo "ANT_PATH is not set in setup.sh"
#    exit 1
#fi

######################################################
# Components
######################################################
export FOCAL_COMMON_INC=${COMP_PATH_ROOT}
#export FOCAL_LIB_LOCATION=/home/henning/projects/tee_release/build/out/gcc
#export FOCAL_LIB_LOCATION=/home/henning/projects/tee/build/out/gcc
export FOCAL_LIB_LOCATION=${COMP_PATH_ROOT}/../focal_lib
#export FOCAL_LIB_BUILD_LOCATION=/home/henning/projects/tee/build/out/gcc
#export FOCAL_LIB_BUILD_LOCATION=$FOCAL_LIB_BUILD_LOCATION
export COMP_PATH_Logwrapper=${COMP_PATH_ROOT}/t-sdk/TlcSdk/Out/Logwrapper


export COMP_PATH_Tools=${COMP_PATH_ROOT}/Tools
#relative path needed for COMP_PATH_MobiCoreDriverLib (multi-OS compatibility for including library)
export COMP_PATH_MobiCore=${COMP_PATH_ROOT}/t-sdk/TlcSdk/Out/Public/
export COMP_PATH_MobiCoreDriverLib=${COMP_PATH_ROOT}/t-sdk/TlcSdk/Out
export COMP_PATH_TlSdk=${COMP_PATH_ROOT}/t-sdk/TlSdk/Out
export COMP_PATH_FOCAL_TA=${COMP_PATH_ROOT}/focal_ta/Out
export COMP_PATH_DrSdk=${COMP_PATH_ROOT}/t-sdk/DrSdk/Out


export COMP_PATH_DR_FOCAL=${COMP_PATH_ROOT}/dr_focal/Out
export COMP_PATH_TAC_FOCAL=${COMP_PATH_ROOT}/tac_focal/Out

export TEE_TRUSTLET_OUTPUT_PATH=$PWD
export TEE_DRIVER_OUTPUT_PATH=$PWD
#export TAC_APP_ABI=armeabi-v7a
export TAC_APP_ABI=arm64-v8a
