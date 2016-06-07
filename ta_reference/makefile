TA_DIR=focal_ta
TAC_DIR=focal_tac
DR_DIR=drfocal
BUILD_DIR=/Locals/Build
CODE_DIR=/Locals/Code
TA_MODE=Debug
TAC_MODE=Debug
DR_MODE=Debug
DR_BIN=/$(DR_MODE)/04020000000000000000000000000000.drbin
DR_BIN2=/$(DR_MODE)/04020000000000000000000000000000.tlbin
TA_BIN=/$(TA_MODE)/04010000000000000000000000000000.tlbin
TAC_BIN=/Out/Bin/$(TAC_MODE)/arm64-v8a/focal_test
TAC_BIN2=/Out/Bin/$(TAC_MODE)/arm64-v8a/sensor_test
TAC_BIN3=/Out/Bin/$(TAC_MODE)/arm64-v8a/libfingerprint_tac.so
TAC_LIB=/Out/Bin/$(TAC_MODE)/libfingerprint_tac.so
DST_FOLDER=/system/app/mcRegistry


all: ta tac 

push_all: all 
push_all: push_ta push_tac 

dr:
	$(DR_DIR)$(BUILD_DIR)/build.sh
	cp $(DR_DIR)/$(DR_MODE)/drfocal.lib $(TA_DIR)$(CODE_DIR)/
	mkdir -p $(TA_DIR)$(CODE_DIR)/drinc
	cp $(DR_DIR)/Out/Public/drfocalApi.h $(TA_DIR)$(CODE_DIR)/drinc
	cp $(DR_DIR)/Out/Public/drTemplate_Api.h $(TA_DIR)$(CODE_DIR)/drinc
	cp $(DR_DIR)/Out/Public/drfocal.h $(TA_DIR)$(CODE_DIR)/drinc

ta:
	$(TA_DIR)$(BUILD_DIR)/build.sh 

tac:
	$(TAC_DIR)$(BUILD_DIR)/build.sh

push_ta: ta
	adb root
	adb remount
	adb push $(TA_DIR)$(TA_BIN) $(DST_FOLDER)


push_tac: tac
	adb push $(TAC_DIR)$(TAC_BIN) /system/bin/
	adb push $(TAC_DIR)$(TAC_BIN2) /system/bin/
	adb push $(TAC_DIR)$(TAC_BIN3) /system/vendor/lib64/

push_dr: dr
	adb push $(DR_DIR)$(DR_BIN) /system/app/mcRegistry
	adb push $(DR_DIR)$(DR_BIN2) /system/app/mcRegistry


clean:	
	rm -Rf $(DR_DIR)/Debug
	rm -Rf $(DR_DIR)/Out
	./$(TA_DIR)$(BUILD_DIR)/build.sh clean
	./$(TAC_DIR)$(BUILD_DIR)/build.sh clean
