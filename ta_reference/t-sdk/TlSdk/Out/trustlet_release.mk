# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Trusted Application Standalone Build)
$(info ******************************************)

# TA_INTERFACE_VERSION : major.minor
ifeq ($(TA_INTERFACE_VERSION),)
$(info - TA_INTERFACE_VERSION  is  not  set,  default  is : 0.0)
TA_INTERFACE_VERSION ?= 0.0
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

ifneq ($(HW_FLOATING_POINT),N)
   ifneq ($(HW_FLOATING_POINT),Y)
      $(error ERROR : HW_FLOATING_POINT value is not correct : $(HW_FLOATING_POINT))
   endif
endif

TA_FLAGS ?= $(TRUSTLET_FLAGS)

# Extended memory layout support is by default for TBASE_API_LEVEL>=5
EXTENDED_LAYOUT = $(shell if [ $(TBASE_API_LEVEL) -ge 5 ] ; then echo Y ; else echo NO ; fi)
TA_HEAP_SIZE_INIT_PARAM :=
TA_HEAP_SIZE_MAX_PARAM :=
ifeq ($(call EXTENDED_LAYOUT),Y)
    TA_FLAGS := $$(($(TA_FLAGS)|8)) 
    ifneq ($(HEAP_SIZE_INIT),)
        TA_HEAP_SIZE_INIT_PARAM :=  -initheapsize $(HEAP_SIZE_INIT)
        ifeq ($(HEAP_SIZE_MAX),)
            TA_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_INIT)
        else
            TA_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_MAX)
        endif    
    endif
else
    ifeq ($(HW_FLOATING_POINT),Y)
        $(error ERROR : require TBASE_API_LEVEL >= 5 to enable HW_FLOATING_POINT)
    endif
endif

OUTPUT_ROOT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)
OUTPUT_PATH := $(OUTPUT_ROOT)/$(TEE_MODE)
TA_UUID ?= $(TRUSTLET_UUID)
TA_UUID := $(shell echo $(TA_UUID) | tr A-Z a-z)
TA_MEMTYPE ?= $(TRUSTLET_MEMTYPE)
TA_NO_OF_THREADS ?= $(TRUSTLET_NO_OF_THREADS)
TA_SERVICE_TYPE ?= $(TRUSTLET_SERVICE_TYPE)
TA_KEYFILE ?= $(TRUSTLET_KEYFILE)
TA_INSTANCES ?= $(TRUSTLET_INSTANCES)
TA_ADD_FLAGS ?= $(TRUSTLET_ADD_FLAGS)
TA_INTERFACE_VERSION ?= 0.0
TASDK_DIR_SRC ?= $(TLSDK_DIR_SRC)
GP_LEVEL :=
GP_UUIDKEYFILE :=
TA_BIN := $(OUTPUT_PATH)/$(TA_UUID).tlbin
TA_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
TA_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
CROSS=arm-none-eabi
READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
READ_OPT=-a $(TA_AXF) > $(TA_LST2)
TA_PARAM := -servicetype $(TA_SERVICE_TYPE) \
            -numberofthreads $(TA_NO_OF_THREADS) \
            -numberofinstances $(TA_INSTANCES) \
            -memtype $(TA_MEMTYPE) \
            -flags $(TA_FLAGS) \
            -bin $(TA_AXF) \
            -output $(TA_BIN) \
            -keyfile $(TA_KEYFILE) \
            $(GP_LEVEL) \
            $(GP_UUIDKEYFILE) \
            -interfaceversion $(TA_INTERFACE_VERSION) \
            $(TA_HEAP_SIZE_INIT_PARAM) \
            $(TA_HEAP_SIZE_MAX_PARAM) \
            $(TA_ADD_FLAGS)

all : $(TA_BIN)

$(TA_BIN) : $(TA_AXF)
	$(info **********************************************************************)
	$(info ** READELF & MobiConvert Standalone - $(OUTPUT_NAME) *****************)
	$(info **********************************************************************)
	$(READELF) $(READ_OPT)
	$(JAVA_HOME)/bin/java -jar $(TASDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(TA_PARAM) >$(TA_BIN).log