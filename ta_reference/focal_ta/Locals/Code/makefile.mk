################################################################################
#
# <t-base Secure SPI driver for fingerprint FTS9300 Si
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := focal_ta

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 04010000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 1
TBASE_API_LEVEL := 5  
HW_FLOATING_POINT := Y
#MODE := Release
#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    Locals/Code/inc \
	$(FOCAL_COMMON_INC) \
	$(FOCAL_LIB_LOCATION) \
    $(COMP_PATH_DR_FOCAL)/Public/ \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc \
	$(COMP_PATH_MobiCore) \
    $(COMP_PATH_TlSdk)/Public \
    Locals/Code/spi/Drspi/Locals/Code/public \
    Locals/Code/spi/Tlspi/Locals/Code/public \
    Locals/Code/spi/Tlcspi/Locals/Code \
    Locals/Code/drinc

### Add source code files for C compiler here
#	
SRC_C += \
    Locals/Code/focal_ta.c \
    Locals/Code/focal_spi.c \
	Locals/Code/focal_db.c \
	Locals/Code/focal_main.c \
	Locals/Code/focal_mem.c\
	Locals/Code/focal_main_test.c\
	Locals/Code/focal_platform.c
	#Locals/Code/focal_fp_spi.c \
	#Locals/Code/quality.c \
	#Locals/Code/focal_sensor.c \
	#Locals/Code/test_main.c
	#Locals/Code/focal_lib_test.c

TRUSTLET_OPTS := -Wall -Wextra

### Add source code files for assembler here
SRC_S += # nothing


HEAP_SIZE_MAX := 16777216
HEAP_SIZE_INIT := 12582912  
include $(BUILD_STATIC_LIBRARY)


#-------------------------------------------------------------------------------
# DrSecureSPI related additions
#-------------------------------------------------------------------------------

ifeq ($(TOOLCHAIN),GNU)
#    CUSTOMER_DRIVER_LIBS += $(COMP_PATH_DR_FOCAL)/Bin/GNU/drfocal.lib
else
#   CUSTOMER_DRIVER_LIBS += $(COMP_PATH_DR_FOCAL)/Bin/drfocal.lib
endif

CUSTOMER_DRIVER_LIBS += \
						Locals/Code/drspi.lib \
						Locals/Code/drutils.lib \
						Locals/Code/drsec.lib \
						Locals/Code/drfocal.lib \
						Locals/Code/libfocal.a 						
						#Locals/Code/focal_lib_test.lib
						
				#	/home/henning/bin/gcc-arm-none-eabi-4_7-2013q1/lib/gcc/arm-none-eabi/4.7.3/armv7-ar/thumb/libgcc.a

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

