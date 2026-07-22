# Select the right arch binary and clean up the rest.
case "$ARCH" in
    arm64) mv -f $MODPATH/bin/arm64-v8a/scmd   $MODPATH/bin/scmd ;;
    arm)   mv -f $MODPATH/bin/armeabi-v7a/scmd $MODPATH/bin/scmd ;;
    x64)  mv -f $MODPATH/bin/x86_64/scmd       $MODPATH/bin/scmd ;;
    *)     abort "unsupported arch: $ARCH" ;;
esac

rm -rf $MODPATH/bin/arm64-v8a $MODPATH/bin/armeabi-v7a $MODPATH/bin/x86_64
