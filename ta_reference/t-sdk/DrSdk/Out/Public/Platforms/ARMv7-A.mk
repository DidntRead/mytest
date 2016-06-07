
#-------------------------------------------------------------------------
#
# ARMv7-A
#
#-------------------------------------------------------------------------

ADD_PLATFORM_TARGETS := ARM_V7A_STD

ifeq ($(PLATFORM),ARM_V7A_STD)

    ARM_VENDOR := ARM

    ARM_CHIP := ARMV7_A
    ARM_ARCH := ARMv7

    ARM_SHAPE := STD

    ARM_SUBDIR := ARMv7-A

    ifeq ($(TOOLCHAIN),GNU)
       ARM_CPU := generic-armv7-a
       ARM_FPU := vfp
    else ifeq ($(TOOLCHAIN),ARM)
       ARM_CPU := 7-A
       ARM_FPU := SoftVFP
    else
       $(error unsupported TOOLCHAIN: $(TOOLCHAIN))
    endif

endif
