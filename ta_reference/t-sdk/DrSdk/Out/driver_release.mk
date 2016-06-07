# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Driver Standalone Build)
$(info ******************************************)

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

OUTPUT_ROOT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)
OUTPUT_PATH := $(OUTPUT_ROOT)/$(TEE_MODE)
DR_BIN := $(OUTPUT_PATH)/$(DRIVER_UUID).drbin
DR_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
DR_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
CROSS=arm-none-eabi
READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
READ_OPT=-a $(DR_AXF) > $(DR_LST2)
DR_PARAM := -servicetype $(DRIVER_SERVICE_TYPE) \
            -numberofthreads $(DRIVER_NO_OF_THREADS) \
            -bin $(DR_AXF) \
            -output $(DR_BIN) \
            -d $(DRIVER_ID) \
            -memtype $(DRIVER_MEMTYPE) \
            -flags $(DRIVER_FLAGS) \
            -interfaceversion $(DRIVER_INTERFACE_VERSION) \
            -keyfile $(DRIVER_KEYFILE) \
            $(DRIVER_HEAP_SIZE_INIT_PARAM) \
            $(DRIVER_HEAP_SIZE_MAX_PARAM)

all : $(DR_BIN)

$(DR_BIN) : $(DR_AXF)
	$(info **********************************************************************)
	$(info ** READELF & MobiConvert Standalone - $(OUTPUT_NAME) *****************)
	$(info **********************************************************************)
	$(READELF) $(READ_OPT)
	$(JAVA_HOME)/bin/java -jar $(DRSDK_DIR_SRC)/Bin/MobiConvert/MobiConvert.jar $(DR_PARAM) >$(DR_BIN).log
	cp $(DR_BIN) $(OUTPUT_PATH)/$(DRIVER_UUID).tlbin
