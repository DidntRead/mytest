################################################################################
#
# <t-sdk SPI Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlspi


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 09150000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10


#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    $(COMP_PATH_Drmem_Export)/public \
    $(COMP_PATH_Drsec_Export)/public \
    $(DRSPI_DIR)/Public/

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlspi.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

ifeq ($(TOOLCHAIN),GNU)
    CUSTOMER_DRIVER_LIBS += \
        $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
    CUSTOMER_DRIVER_LIBS += \
        $(DRUTILS_LIB_PATH)    
else
   CUSTOMER_DRIVER_LIBS += \
        $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
   CUSTOMER_DRIVER_LIBS += \
        $(DRUTILS_LIB_PATH)    
endif

CUSTOMER_DRIVER_LIBS += $(DRSEC_LIB_PATH)

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

