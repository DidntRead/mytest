# =============================================================================
#
# MobiCore log wrapper to be included by Android components / products
#
# =============================================================================

# This is not a separate module.
# Only for inclusion by other modules.

LOCAL_LDLIBS += -llog

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID

ifeq ($(APP_ABI),arm64-v8a)
    LOCAL_CFLAGS += -fno-stack-protector
endif

LOCAL_C_INCLUDES += $(call my-dir)
