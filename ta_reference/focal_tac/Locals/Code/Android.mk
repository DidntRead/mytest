# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)
TBASE_API_LEVEL := 5
# =============================================================================

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= focal_tac

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	Locals/Code/inc \
	$(FOCAL_COMMON_INC) \
	$(FOCAL_LIB_LOCATION) \
	$(COMP_PATH_FOCAL_TA)/Public \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc \
	$(COMP_PATH_MobiCore)/inc \
        $(COMP_PATH_MobiCoreDriverLib)

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= focal_tac.c 

LOCAL_C_FLAGS := -Wall -Wextra
# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := MobiCoreDriver

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)

# =============================================================================
include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= focal_hal 

# Add your folders with header files here
LOCAL_C_INCLUDES += Locals/Code/hal_extension/include \
					Locals/Code/inc \
					$(FOCAL_COMMON_INC) \
					$(FOCAL_LIB_LOCATION) \
                     $(COMP_PATH_MobiCoreDriverLib)

# Add your source files here (relative paths)
LOCAL_SRC_FILES	= focal_hal.c \
					hal_extension/lib_interface.c \
					hal_extension/sensor_irq.c \
					hal_extension/util.c \
					hal_extension/sensor.c \
					hal_extension/sensor_ctrl.c


LOCAL_C_FLAGS := -Wall -Wextra

LOCAL_STATIC_LIBRARIES := focal_tac

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_STATIC_LIBRARY)
# =============================================================================
# JNI module: libfingerprint_tac.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= libfingerprint_tac

LOCAL_C_INCLUDES += \
			Locals/Code/inc \
			Locals/Code/hal_extension/include \
			$(JNI_H_INCLUDE)\
			$(FOCAL_LIB_LOCATION) \
			$(FOCAL_COMMON_INC) \
			$(COMP_PATH_TA_FOCAL)/Public \
			$(ANDROID_INCLUDE) \
                        $(COMP_PATH_MobiCoreDriverLib)

# Add new source files here
LOCAL_SRC_FILES	+= focal_jni.c
#Java/focal_jni.cpp 
LOCAL_C_FLAGS := -Wall

LOCAL_STATIC_LIBRARIES := focal_tac focal_hal 

LOCAL_CFLAGS := -DNDK_ROOT
#LOCAL_SHARED_LIBRARIES := MobiCoreDriver
include $(COMP_PATH_Logwrapper)/Android.mk

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)


# =============================================================================
# Fingerprint test binary

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= focal_test

LOCAL_C_INCLUDES += Locals/Code/inc \
					$(FOCAL_LIB_LOCATION) \
					$(FOCAL_COMMON_INC)

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= focal_test.c

LOCAL_C_FLAGS := -Wall
LOCAL_STATIC_LIBRARIES := focal_tac focal_hal

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)

# =============================================================================
# Fingerprint test binary

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= focal_ta_test 

LOCAL_C_INCLUDES += Locals/Code/inc \
					$(FOCAL_LIB_LOCATION) \
					$(FOCAL_COMMON_INC)

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= sensor_test.c

LOCAL_C_FLAGS := -Wall
LOCAL_STATIC_LIBRARIES := focal_tac focal_hal 

include $(COMP_PATH_Logwrapper)/Android.mk

include $(BUILD_EXECUTABLE)
# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
