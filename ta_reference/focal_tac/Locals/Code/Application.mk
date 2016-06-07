# =============================================================================
#
# Main build file defining the project modules and their global variables.
#
# =============================================================================

# Don't remove this - mandatory
APP_PROJECT_PATH := $(call my-dir)
#APP_ABI := armeabi-v7a
APP_ABI := arm64-v8a

# The only STL implementation currently working with exceptions
#APP_STL := gnustl_static

# Don't optimize for better debugging
APP_OPTIM := debug
