# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Driver Build)
$(info ******************************************)

# TEE_MODE : Debug, Release
ifeq ($(PLATFORM),)
$(info - PLATFORM  is  not  set,  default  is : ARM_VE_A9X4_STD)
PLATFORM ?= ARM_V7A_STD
endif

# TEE_MODE : Debug, Release
ifeq ($(TEE_MODE),)
$(info - MODE  is  not  set,  default  is : Debug)
TEE_MODE ?= Debug
endif

# TOOLCHAIN : ARM, GNU
ifeq ($(TOOLCHAIN),)
$(info - TOOLCHAIN is not set, default is : GNU)
TOOLCHAIN ?= GNU
endif

# TBASE_API_LEVEL
ifeq ($(TBASE_API_LEVEL),)
$(info - TBASE_API_LEVEL  is  not  set,  default  is : 2)
TBASE_API_LEVEL ?= 2
endif

# HW_FLOATING_POINT
ifeq ($(HW_FLOATING_POINT),)
$(info - HW_FLOATING_POINT  is  not  set,  default  is : N)
HW_FLOATING_POINT ?= N
endif

# TEE_MACH_TYPE
ifeq ($(TEE_MACH_TYPE),)
$(info - TEE_MACH_TYPE  is  not  set,  default  is : MTXXXX)
TEE_MACH_TYPE ?= MTXXXX
endif

$(info ******************************************)
$(info PLATFORM : $(MTK_PROJECT))
$(info TOOLCHAIN : $(TOOLCHAIN))
$(info MODE      : $(TEE_MODE))
$(info TBASE_API_LEVEL : $(TBASE_API_LEVEL))
$(info HW_FLOATING_POINT : $(HW_FLOATING_POINT))
$(info TEE_MACH_TYPE : $(TEE_MACH_TYPE))
$(info ******************************************)

# Check if variables are correctly set
ifneq ($(TEE_MODE),Debug)
   ifneq ($(TEE_MODE),Release)
      $(error ERROR : MODE value is not correct : $(TEE_MODE))
   endif
endif
ifneq ($(TOOLCHAIN),GNU)
   ifneq ($(TOOLCHAIN),ARM)
      $(error ERROR : TOOLCHAIN value is not correct : $(TOOLCHAIN))
   endif
endif

# Extended memory layout support is by default for TBASE_API_LEVEL>=5
EXTENDED_LAYOUT = $(shell if [ $(TBASE_API_LEVEL) -ge 5 ] ; then echo Y ; else echo NO ; fi)
DRIVER_HEAP_SIZE_INIT_PARAM :=
DRIVER_HEAP_SIZE_MAX_PARAM :=
ifeq ($(call EXTENDED_LAYOUT),Y)
    DRIVER_FLAGS := $$(($(DRIVER_FLAGS)|8)) 
    ifneq ($(HEAP_SIZE_INIT),)
        DRIVER_HEAP_SIZE_INIT_PARAM :=  -initheapsize $(HEAP_SIZE_INIT)
        ifeq ($(HEAP_SIZE_MAX),)
            DRIVER_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_INIT)
        else
            DRIVER_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_MAX)
        endif    
    endif
else
    ifeq ($(HW_FLOATING_POINT),Y)
        $(error ERROR : require TBASE_API_LEVEL >= 5 to enable HW_FLOATING_POINT)
    endif
endif

ifneq ($(HW_FLOATING_POINT),N)
   ifneq ($(HW_FLOATING_POINT),Y)
      $(error ERROR : HW_FLOATING_POINT value is not correct : $(HW_FLOATING_POINT))
   endif
endif

ifeq ($(TOOLCHAIN),GNU)
   CROSS=arm-none-eabi
   #CROSS=aarch64-linux-gnu
   LINKER_SCRIPT=$(DRSDK_DIR_SRC)/driver.ld

   CC=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   LINKER=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ld
   READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
   #ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-as
   #We use gcc as preprocessor for GNU asm 	
   ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc   
   ARCH=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ar
else
   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
   ARCH=$(ARM_RVCT_PATH_BIN)/armar
endif

# OUTPUT_ROOT used to clean build, keep it defined
OUTPUT_ROOT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)
ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(TEE_MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(TEE_MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
endif
OUTPUT_OBJ_PATH=$(OUTPUT_PATH)/obj-local
OUTPUT_PLAT_OBJ_PATH=$(OUTPUT_PATH)/obj-plat

# ************************************************************************************************
# Driver being built
DR_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
DR_BIN := $(OUTPUT_PATH)/$(DRIVER_UUID).drbin
DR_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
DR_LIB := $(OUTPUT_PATH_INDEPENDENT)/$(OUTPUT_NAME).lib

# ************************************************************************************************
# DRSDK
DRSDK_SRC_C := 
DRSDK_SRC_ASM := 
DRSDK_DIR_INC := $(DRSDK_DIR_SRC)/Public $(DRSDK_DIR_SRC)/Public/MobiCore/inc $(TLSDK_DIR_SRC)/Public  $(TLSDK_DIR_SRC)/Public/MobiCore/inc
DRAPI_LIB := $(DRSDK_DIR_SRC)/Bin/DrApi/DrApi.lib 
DRENTRY_LIB := $(DRSDK_DIR_SRC)/Bin/DrEntry/DrEntry.lib


# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Get architecture specifics files from platform makefile.
#  These values end up in compiler settings and provide handy defines in your source files.
include $(DRSDK_DIR_SRC)/Public/Platforms/*.mk

ARM_PLAT := $(ARM_CHIP)_$(ARM_SHAPE)

# ************************************************************************************************
# CC options
ARM_OPT_CC := # init as empty 
ARM_OPT_CC += -DPLAT=$(ARM_PLAT)
ARM_OPT_CC += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
ARM_OPT_CC += -D__$(ARM_CHIP)__
ARM_OPT_CC += -D__$(ARM_PLAT)__
ARM_OPT_CC += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE) 
ARM_OPT_CC += -DTBASE_API_LEVEL=$(TBASE_API_LEVEL)
ARM_OPT_CC += -DTEE_MACH_TYPE_$(shell echo $(TEE_MACH_TYPE) | tr a-z A-Z)

CPU_OPT_CC_NEON :=
CPU_OPT_CC_NO_NEON :=
CPU_OPT_CC_NO_NEON_LIB :=
ifeq ($(TOOLCHAIN),GNU)
    CPU_OPT_CC_NO_NEON += -mcpu=$(ARM_CPU)
    CPU_OPT_CC_NO_NEON += -mfpu=vfp
    CPU_OPT_CC_NO_NEON += -mfloat-abi=soft

    CPU_OPT_CC_NO_NEON_LIB += -mcpu=generic-armv7-a
    CPU_OPT_CC_NO_NEON_LIB += -mfpu=vfp
    CPU_OPT_CC_NO_NEON_LIB += -mfloat-abi=soft

    CPU_OPT_CC_NEON += -mcpu=$(ARM_CPU)
    CPU_OPT_CC_NEON += -mfpu=neon-vfpv4
    CPU_OPT_CC_NEON += -mfloat-abi=softfp
else
    CPU_OPT_CC_NO_NEON += --cpu=cortex-a15
	CPU_OPT_CC_NO_NEON += --fpu=SoftVFP

    CPU_OPT_CC_NO_NEON_LIB += --cpu=7-A
	CPU_OPT_CC_NO_NEON_LIB += --fpu=SoftVFP

    CPU_OPT_CC_NEON += --cpu=cortex-a15
    CPU_OPT_CC_NEON += --fpu=VFPv4

    # Special treatment for --diag_error=warnings, downgrade compiler warnings to remarks
    # Suppress Error: C9931W: Your license for Compiler (feature compiler) will expire in 28 days
    ARM_OPT_CC += --diag_remark=9931
    # Suppress Error: C9933W: Waiting for license...
    ARM_OPT_CC += --diag_remark=9933
    # Treat "#223-D: function declared implicitly" as an error
    ARM_OPT_CC += --diag_error=223
endif

ARM_OPT_ASM := # init as empty
ifeq ($(TOOLCHAIN),GNU)
    ARM_OPT_ASM += -DPLAT=$(ARM_PLAT)
    ARM_OPT_ASM += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
    ARM_OPT_ASM += -D__$(ARM_CHIP)__
    ARM_OPT_ASM += -D__$(ARM_PLAT)__
    ARM_OPT_ASM += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)
    ARM_OPT_ASM += -DTEE_MACH_TYPE_$(shell echo $(TEE_MACH_TYPE) | tr a-z A-Z)
else
    ARM_OPT_ASM += --cpreproc_opts='--c90,-DPLAT=$(ARM_PLAT),-DARM_ARCH=$(ARM_ARCH),-D__$(ARM_ARCH)__,-D__$(ARM_CHIP)__,-D__$(ARM_PLAT)__,-D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)'
endif

ifeq ($(HW_FLOATING_POINT),Y)
    ARM_OPT_ASM += $(CPU_OPT_CC_NEON)
else
    ARM_OPT_ASM += $(CPU_OPT_CC_NO_NEON)
endif

# ************************************************************************************************
# Debug options

ifeq ($(TEE_MODE),Debug)
   CC_DBG_OPTS := -DDEBUG --debug

ifeq ($(TOOLCHAIN),GNU)
       ASM_DBG_OPTS :=  -Wa,-g,--keep-locals
else
   ASM_DBG_OPTS :=  -g \
                    --keep
endif

   ifeq ($(TOOLCHAIN),GNU)
      LINK_DBG_OPTS :=
   else
      LINK_DBG_OPTS := --debug
   endif

else
   ifeq ($(TOOLCHAIN),GNU)
      #TBUG-400 LINK_DBG_OPTS := --strip-debug
   else
      LINK_DBG_OPTS := --no_debug --no_comment_section
   endif

endif

# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Standard options

DR_PARAM := -servicetype $(DRIVER_SERVICE_TYPE) \
            -numberofthreads $(DRIVER_NO_OF_THREADS) \
            -bin $(DR_AXF) \
            -output $(DR_BIN) \
            -d $(DRIVER_ID) \
            -memtype $(DRIVER_MEMTYPE) \
            -flags $(DRIVER_FLAGS) \
            -interfaceversion $(DRIVER_INTERFACE_VERSION) \
            $(DRIVER_HEAP_SIZE_INIT_PARAM) \
            $(DRIVER_HEAP_SIZE_MAX_PARAM)


ifeq ($(DRIVER_KEYFILE),)
DR_PARAM += --nosign
else
DR_PARAM += -keyfile $(DRIVER_KEYFILE)
endif


ifeq ($(TOOLCHAIN),GNU)
CC_OPTS_BASE :=\
            -mthumb \
            -O3 \
            -mthumb-interwork

ifneq ($(ARMCC_SHORT_ENUMS),YES)
CC_OPTS_BASE += -fno-short-enums
endif

else
CC_OPTS_BASE := -D__ARMCC__ \
            --thumb \
            --apcs=interwork \
            --reduce_paths \
            --diag_style=gnu \
            --no_depend_system_headers \
            --gnu \
            --bss_threshold=0 \
            --interface_enums_are_32_bit \
            -O3 \
            -Ospace \
            -J$(ARM_RVCT_PATH_INC)

ifneq ($(ARMCC_SHORT_ENUMS),YES)
CC_OPTS_BASE += --enum_is_int
endif

# Avoid optimized memcpy in struct accesses etc, to avoid undef ref __aeabi_memmove etc when compiling with gcc
CC_OPTS_LIB_SPECIFIC := --library_interface=none
endif

CC_OPTS_BASE += \
            -D__THUMB__ \
            $(ARM_OPT_CC) \
            $(ARMCC_COMPILATION_FLAGS)

# NOTE: The TA-TDriver library should be compiled without floating-point,
#       if no (float) in the interface, otherwise the compiler complains 
#       when compiling the TA, if the TA does not use FP itself.
#       Here the driver developer can define its own cpu settings for the lib.
CPU_OPT_CC_LIB ?= $(CPU_OPT_CC_NO_NEON_LIB)

CC_OPTS_LIB := \
            $(CC_OPTS_BASE) \
            $(CPU_OPT_CC_LIB) \
            $(CC_OPTS_LIB_SPECIFIC)

CC_OPTS :=  $(CC_OPTS_BASE) \
            $(CC_DBG_OPTS)

ifeq ($(HW_FLOATING_POINT),Y)
    CC_OPTS += $(CPU_OPT_CC_NEON)
else
    CC_OPTS += $(CPU_OPT_CC_NO_NEON)
endif

CC_OPTS += -DDRIVER

ifeq ($(TOOLCHAIN),GNU)
	ASM_OPTS := -mthumb-interwork \
                -D__THUMB__ \
                $(ARM_OPT_ASM) \
                ${ARMCC_COMPILATION_FLAGS} \
                $(ASM_DBG_OPTS) \
                -c

else
    ASM_OPTS := --arm \
                --apcs=interwork \
                --reduce_paths \
                --diag_style=gnu \
                --cpreproc \
                --cpreproc_opts='-D__ARMCC__,-D__THUMB__' \
                $(ARM_OPT_ASM) \
                $(ASM_DBG_OPTS)
    comma:= ,
    empty:=
    space:= $(empty) $(empty)
    tmp = $(subst $(space),$(comma),$(strip $(ARMCC_COMPILATION_FLAGS)))
    ifneq ($(tmp),)
        ASM_OPTS += --cpreproc_opts='$(tmp)'
    endif
endif

STD_LIBS ?=
ifeq ($(TOOLCHAIN),GNU)
LINK_OPTS := --verbose \
            -nostdlib \
            --gc-sections \
            -T $(LINKER_SCRIPT) \
            -o $(DR_AXF) \
            $(LINK_DBG_OPTS) \
            -L $(CROSS_GCC_PATH_LIB) \
            -L $(CROSS_GCC_PATH_LGCC) \
            --fatal-warnings \
            --no-wchar-size-warning

STD_LIBS += -lm -lc -lgcc
else
LINK_OPTS := --entry _drEntry \
             --reduce_paths \
             --diag_style=gnu \
             --datacompressor=off \
             --verbose \
             --map \
             --callgraph \
             --remove \
             --symbols \
             --list=$(OUTPUT_PATH)/$(OUTPUT_NAME).lst \
             --libpath=$(ARM_RVCT_PATH_LIB) \
             --scatter=$(DRSDK_DIR_SRC)/dr.sct \
             -o $(DR_AXF) \
             $(LINK_DBG_OPTS) \
             $(EXTRA_LINK_OPTS)
endif

ifeq ($(TOOLCHAIN),GNU)
ARCHIVER_OPTS := -rcs
else
ARCHIVER_OPTS := --create \
             --debug_symbols \
             -v
endif
			
# ************************************************************************************************
# Actual sets of files to work on
PLAT_SRC := $(DRSDK_DIR_SRC)/Platform/DrApiPlat.c
SRC := $(SRC_C) $(SRC_CPP) $(SRC_ASM)
DRSDK_SRC := $(DRSDK_SRC_C) $(DRSDK_SRC_ASM)

SRC_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(join $(dir $(SRC)), $(addsuffix .o,$(notdir $(basename $(SRC))))))
PLAT_OBJ = $(addprefix $(OUTPUT_PLAT_OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(PLAT_SRC)))))
DRSDK_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .o,$(notdir $(basename $(DRSDK_SRC))))) 
OBJ := $(SRC_OBJ) $(DRSDK_OBJ) $(PLAT_OBJ)

ifneq ($(strip $(SRC_LIB_C)),)
DR_LIB_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .ol,$(notdir $(basename $(SRC_LIB_C)))))
endif


ifeq ($(TOOLCHAIN),GNU)
   INC := $(INCLUDE_DIRS:%=-I%) $(DRSDK_DIR_INC:%=-I%) -I$(CROSS_GCC_PATH_INC)
else
   INC := $(INCLUDE_DIRS:%=-I%) $(DRSDK_DIR_INC:%=-I%)
endif

LIBS := $(DRAPI_LIB) $(DRENTRY_LIB) $(EXTRA_LIBS)
#LIBS += $(CROSS_GCC_PATH_LIB_M) $(CROSS_GCC_PATH_LIB_C)
#LIBS += $(CROSS_GCC_PATH_LIB_GCC)

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT=-a $(DR_AXF) > $(DR_LST2)
   C99=-std=c99
else
   READ_OPT=--text -c -d -e -t -z --datasymbols $(DR_AXF) --output $(DR_LST2)
   C99=--c99
endif

# ************************************************************************************************
# Rules

ifdef DR_LIB_OBJ
all : $(DR_BIN) $(DR_LIB)
else
all : $(DR_BIN)
endif

$(DR_AXF) : $(OBJ) $(LIBS)
	$(info ******************************************)
	$(info ** LINKER ********************************)
	$(info ******************************************)
	$(LINKER) $(LINK_OPTS) $(OBJ) $(LIBS) $(STD_LIBS)

$(DR_BIN) : $(DR_AXF)
ifeq ($(TRUSTLET_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	$(JAVA_HOME)/bin/java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(DR_MOBICONFIG_PARAM)
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT)
	$(JAVA_HOME)/bin/java -jar $(DRSDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(DR_PARAM) >$(DR_BIN).log
ifeq ($(DRIVER_KEYFILE),)
	mv $(DR_BIN).raw $(DR_BIN)
endif
	cp $(DR_BIN) $(OUTPUT_PATH)/$(DRIVER_UUID).tlbin


$(DR_LIB) : $(DR_LIB_OBJ)
	$(ARCH) $(ARCHIVER_OPTS) $(DR_LIB) $(DR_LIB_OBJ) 
	
.PHONY: clean

clean:
	rm -rf $(OUTPUT_PATH)/*

clean_all:
	rm -rf $(OUTPUT_ROOT)/*/*/*

$(OUTPUT_PLAT_OBJ_PATH)/%.o : $(DRSDK_DIR_SRC)/Platform/%.c
	mkdir -p $(dir $@)
	$(CC) $(C99) $(CC_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(C99) $(CC_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) --cpp $(CC_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.ol : $(CURDIR)/$(DRLIB_DIR)/%.c
	mkdir -p $(OUTPUT_PATH_INDEPENDENT)
	$(CC) $(C99) $(CC_OPTS_LIB) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(INC) -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.S
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(INC) -o $@ $<
# ************************************************************************************************

