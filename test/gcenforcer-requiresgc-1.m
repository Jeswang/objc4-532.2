// gc-off app loading gc-required dylib: should crash
// linker sees librequiresgc.fake.dylib, runtime uses librequiresgc.dylib

/*
TEST_CONFIG MEM=mrc,arc SDK=macos
TEST_CRASHES

TEST_RUN_OUTPUT
objc\[\d+\]: '.*librequiresgc.dylib' was compiled with -fobjc-gc-only, but the application does not support GC
objc\[\d+\]: \*\*\* GC capability of application and some libraries did not match
CRASHED: SIGILL
END

TEST_BUILD
    $C{COMPILE_C} $DIR/gc.c -dynamiclib -o libnoobjc.dylib
    $C{COMPILE_NOMEM} $DIR/gc.m -dynamiclib -o libnogc.dylib
    $C{COMPILE_NOMEM} $DIR/gc.m -dynamiclib -o libsupportsgc.dylib -fobjc-gc
    $C{COMPILE_NOMEM} $DIR/gc.m -dynamiclib -o librequiresgc.dylib -fobjc-gc-only
    $C{COMPILE_NOMEM} $DIR/gc.m -dynamiclib -o librequiresgc.fake.dylib -fobjc-gc -install_name librequiresgc.dylib

    $C{COMPILE} $DIR/gc-main.m -x none librequiresgc.fake.dylib -o gcenforcer-requiresgc-1.out
END
*/
