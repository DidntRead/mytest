
#-------------------------------------------------------------------------
#
# Samsung ARMv7 platforms
#
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# generic variables

# supported targets
ADD_PLATFORM_TARGETS := \
    EXYNOS_4X12_STD \
    EXYNOS_5250_STD \
    EXYNOS_5250_LPAE \
    EXYNOS_5410_STD \
    EXYNOS_5420_STD \
    EXYNOS_5422_STD \
    EXYNOS_5430_STD \
    EXYNOS_5260_STD \
    EXYNOS_3470_STD \
    EXYNOS_4415_STD


#-------------------------------------------------------------------------
# specific platforms
VALID_PLATFORM = $(empty)

# Exynos4X12 (Pegasus dual/quad-core)
ifeq ($(PLATFORM),EXYNOS_4X12_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_4X12
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 4
    PLATFORM_CLUSTER_COUNT := 1

    ARM_CPU := Cortex-A9.no_neon.no_vfp

ifeq ($(MODE),Debug)
    PLATFORM_FEATURES := COMPRESSED_CORESYSTEM
endif
endif

# Exynos5250 (Gaia dual-core)
ifeq ($(PLATFORM),EXYNOS_5250_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5250
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 2
    PLATFORM_CLUSTER_COUNT := 1
    PLATFORM_FEATURES := COMPRESSED_CORESYSTEM

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Same as EXYNOS_5250_STD except that LPAE is activated in the SWd
ifeq ($(PLATFORM),EXYNOS_5250_LPAE)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5250
    ARM_SHAPE := LPAE
    PLATFORM_CORE_COUNT := 2
    PLATFORM_CLUSTER_COUNT := 1
    PLATFORM_FEATURES := COMPRESSED_CORESYSTEM

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos5410 (Adonis big.LITTLE - quad A15, quad A7)
ifeq ($(PLATFORM),EXYNOS_5410_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5410
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 8
    PLATFORM_CLUSTER_COUNT := 2

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos5420 (Ares big.LITTLE - quad A15, quad A7)
ifeq ($(PLATFORM),EXYNOS_5420_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5420
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 8
    PLATFORM_CLUSTER_COUNT := 2

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos5422 (Ares big.LITTLE - quad A15, quad A7, HMP)
ifeq ($(PLATFORM),EXYNOS_5422_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5422
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 8
    PLATFORM_CLUSTER_COUNT := 2

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos5430 (Helsinki big.LITTLE - quad A15, quad A7)
ifeq ($(PLATFORM),EXYNOS_5430_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5430
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 8
    PLATFORM_CLUSTER_COUNT := 2

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos5260 (Rhea big.LITTLE - dual A15, quad A7)
ifeq ($(PLATFORM),EXYNOS_5260_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_5260
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 8
    PLATFORM_CLUSTER_COUNT := 2

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos3470 (Carmen EVT1 Quad Core A7)
ifeq ($(PLATFORM),EXYNOS_3470_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_3470
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 4
    PLATFORM_CLUSTER_COUNT := 1

    ARM_CPU := Cortex-A15.no_neon.no_vfp
endif

# Exynos4415 (Hudson quad-core)
ifeq ($(PLATFORM),EXYNOS_4415_STD)
    VALID_PLATFORM = TRUE
    ARM_CHIP  := EXYNOS_4415
    ARM_SHAPE := STD
    PLATFORM_CORE_COUNT := 4
    PLATFORM_CLUSTER_COUNT := 1

    ARM_CPU := Cortex-A9.no_neon.no_vfp
endif

#-------------------------------------------------------------------------
# platform generic
ifeq ($(VALID_PLATFORM),TRUE)
    ARM_VENDOR := Samsung
    ARM_ARCH   := ARMv7
    ARM_SUBDIR := Exynos

    ifeq ($(TOOLCHAIN),GNU)
       ARM_CPU := cortex-a9
       ARM_FPU := vfp
    else ifeq ($(TOOLCHAIN),ARM)
       ARM_FPU := SoftVFP
    else
       $(error unsupported TOOLCHAIN: $(TOOLCHAIN))
    endif

endif
