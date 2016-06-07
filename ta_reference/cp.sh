

DEBUG_DIR=/home/alipay/trustonic_focal_release_1.0/ta_reference

rm -rf /mnt/hgfs/alipay/alipay/TA/tac/armeabi-v7a/*
rm -rf /mnt/hgfs/alipay/alipay/TA/tac/arm64-v8a/*

cp -rf $DEBUG_DIR/focal_tac/Out/Bin/Debug/arm64-v8a/* //mnt/hgfs/alipay/alipay/TA/tac/arm64-v8a
cp -rf $DEBUG_DIR/focal_tac/Out/Bin/Debug/armeabi-v7a/* /mnt/hgfs/alipay/alipay/TA/tac/armeabi-v7a
rm -rf /mnt/hgfs/alipay/alipay/TA/ta/*

cp -rf $DEBUG_DIR/focal_ta/Debug/*  //mnt/hgfs/alipay/alipay/TA/ta/
