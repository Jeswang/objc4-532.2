/* 
llvm-gcc unavoidably warns about our deliberately out-of-order handlers

TEST_BUILD_OUTPUT
.*exc.m: In function .*
.*exc.m:\d+: warning: exception of type .* will be caught
.*exc.m:\d+: warning:    by earlier handler for .*
.*exc.m:\d+: warning: exception of type .* will be caught
.*exc.m:\d+: warning:    by earlier handler for .*
.*exc.m:\d+: warning: exception of type .* will be caught
.*exc.m:\d+: warning:    by earlier handler for .*
OR
END
*/

#include "test.h"
#include "testroot.i"
#include <objc/runtime.h>
#include <objc/objc-exception.h>

static volatile int state = 0;
#define BAD 1000000

#if defined(USE_FOUNDATION)

#include <Foundation/Foundation.h>

@interface Super : NSException @end
@implementation Super
+(id)exception { return AUTORELEASE([[self alloc] initWithName:@"Super" reason:@"reason" userInfo:nil]);  }
-(void)check { state++; }
+(void)check { testassert(!"caught class object, not instance"); }
@end

#define FILENAME "nsexc.m"

#else

@interface Super : TestRoot @end
@implementation Super
+(id)exception { return AUTORELEASE([self new]); }
-(void)check { state++; }
+(void)check { testassert(!"caught class object, not instance"); }
@end

#define FILENAME "exc.m"

#endif

@interface Sub : Super @end
@implementation Sub 
@end


#if __OBJC2__  &&  !TARGET_OS_EMBEDDED  &&  !TARGET_OS_IPHONE
void altHandlerFail(id unused __unused, void *context __unused)
{
    fail("altHandlerFail called");
}

#define ALT_HANDLER(n)                                          \
    void altHandler##n(id unused __unused, void *context)       \
    {                                                           \
        testassert(context == (void*)&altHandler##n);           \
        testassert(state == n);                                 \
        state++;                                                \
    }

ALT_HANDLER(1)
ALT_HANDLER(2)
ALT_HANDLER(3)
ALT_HANDLER(4)
ALT_HANDLER(5)
ALT_HANDLER(6)
ALT_HANDLER(7)


static void throwWithAltHandler(void) __attribute__((noinline));
static void throwWithAltHandler(void)
{
    @try {
        state++;
        uintptr_t token = objc_addExceptionHandler(altHandler3, (void*)altHandler3);
        // state++ inside alt handler
        @throw [Super exception];
        state = BAD;
        objc_removeExceptionHandler(token);
    } 
    @catch (Sub *e) {
        state = BAD;
    }
    state = BAD;
}


static void throwWithAltHandlerAndRethrow(void) __attribute__((noinline));
static void throwWithAltHandlerAndRethrow(void)
{
    @try {
        state++;
        uintptr_t token = objc_addExceptionHandler(altHandler3, (void*)altHandler3);
        // state++ inside alt handler
        @throw [Super exception];
        state = BAD;
        objc_removeExceptionHandler(token);
    } 
    @catch (...) {
        testassert(state == 4);
        state++;
        @throw;
    }
    state = BAD;
}

#endif

#if __cplusplus  &&  __OBJC2__
#include <exception>
void terminator() {
    succeed(FILENAME);    
}
#endif


int main()
{
    PUSH_POOL {

        testprintf("try-catch-finally, exception caught exactly\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Super *e) {
                state++;
                [e check];  // state++
            }
            @finally {
                state++;
            }
            state++;
        } 
        @catch (...) {
            state = BAD;
        }
        testassert(state == 6);
        
        
        testprintf("try-finally, no exception thrown\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
            } 
            @finally {
                state++;
            }
            state++;
        } 
        @catch (...) {
            state = BAD;
        }
        testassert(state == 4);
        
        
        testprintf("try-finally, with exception\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @finally {
                state++;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 5);
        
        
        testprintf("try-catch-finally, no exception\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
            } 
            @catch (...) {
                state = BAD;
            }
            @finally {
                state++;
            }
            state++;
        } @catch (...) {
            state = BAD;
        }
        testassert(state == 4);
        
        
        testprintf("try-catch-finally, exception not caught\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Sub *e) {
                state = BAD;
            }
            @finally {
                state++;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 5);
        
        
        testprintf("try-catch-finally, exception caught exactly, rethrown\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Super *e) {
                state++;
                [e check];  // state++
                @throw;
                state = BAD;
            }
            @finally {
                state++;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 7);
        
        
        testprintf("try-catch, no exception\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
            } 
            @catch (...) {
                state = BAD;
            }
            state++;
        } @catch (...) {
            state = BAD;
        }
        testassert(state == 3);
        
        
        testprintf("try-catch, exception not caught\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Sub *e) {
                state = BAD;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 4);
        
        
        testprintf("try-catch, exception caught exactly\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Super *e) {
                state++;
                [e check];  // state++
            }
            state++;
        } 
        @catch (...) {
            state = BAD;
        }
        testassert(state == 5);
        
        
        testprintf("try-catch, exception caught exactly, rethrown\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Super *e) {
                state++;
                [e check];  // state++
                @throw;
                state = BAD;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 6);
        
        
        testprintf("try-catch, exception caught exactly, thrown again explicitly\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (Super *e) {
                state++;
                [e check];  // state++
                @throw e;
                state = BAD;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 6);
        
        
        testprintf("try-catch, default catch, rethrown\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (...) {
                state++;
                @throw;
                state = BAD;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 5);
        
        
        testprintf("try-catch, default catch, rethrown and caught inside nested handler\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (...) {
                state++;
                
                @try {
                    state++;
                    @throw;
                    state = BAD;
                } @catch (Sub *e) {
                    state = BAD;
                } @catch (Super *e) {
                    state++;
                    [e check];  // state++
                } @catch (...) {
                    state = BAD;
                } @finally {
                    state++;
                }
                
                state++;
            }
            state++;
        } 
        @catch (...) {
            state = BAD;
        }
        testassert(state == 9);
        
        
        testprintf("try-catch, default catch, rethrown inside nested handler but not caught\n");
        
        state = 0;
        @try {
            state++;
            @try {
                state++;
                @throw [Super exception];
                state = BAD;
            } 
            @catch (...) {
                state++;
                
                @try {
                    state++;
                    @throw;
                    state = BAD;
                } 
                @catch (Sub *e) {
                    state = BAD;
                } 
                @finally {
                    state++;
                }
                
                state = BAD;
            }
            state = BAD;
        } 
        @catch (id e) {
            state++;
            [e check];  // state++
        }
        testassert(state == 7);


#if __cplusplus  &&  __OBJC2__
        testprintf("C++ try/catch, Objective-C exception superclass\n");

        state = 0;
        try {
            state++;
            try {
                state++;
                try {
                    state++;
                    @throw [Super exception];
                    state = BAD;
                } catch (...) {
                    state++;
                    throw;
                    state = BAD;
                }
                state = BAD;
            } catch (void *e) {
                state = BAD;
            } catch (int e) {
                state = BAD;
            } catch (Sub *e) {
                state = BAD;
            } catch (Super *e) {
                state++;
                [e check];  // state++
                throw;
            } catch (...) {
                state = BAD;
            }
        } catch (id e) {
            state++;
            [e check];  // state++;
        }
        testassert(state == 8);


        testprintf("C++ try/catch, Objective-C exception subclass\n");

        state = 0;
        try {
            state++;
            try {
                state++;
                try {
                    state++;
                    @throw [Sub exception];
                    state = BAD;
                } catch (...) {
                    state++;
                    throw;
                    state = BAD;
                }
                state = BAD;
            } catch (void *e) {
                state = BAD;
            } catch (int e) {
                state = BAD;
            } catch (Super *e) {
                state++;
                [e check];  // state++
                throw;
            } catch (Sub *e) {
                state = BAD;
            } catch (...) {
                state = BAD;
            }
        } catch (id e) {
            state++;
            [e check];  // state++;
        }
        testassert(state == 8);

#endif        
        
        
#if !__OBJC2__  ||  TARGET_OS_EMBEDDED  ||  TARGET_OS_IPHONE
        // alt handlers for modern Mac OS only

#else
        // alt handlers
        // run a lot to catch failed unregistration (runtime complains at 1000)
#define ALT_HANDLER_REPEAT 2000
        int i;
        
        testprintf("alt handler, no exception\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    state++;
                    @try {
                        uintptr_t token = objc_addExceptionHandler(altHandlerFail, 0);
                        state++;
                        objc_removeExceptionHandler(token);
                    } 
                    @catch (...) {
                        state = BAD;
                    }
                    state++;
                } @catch (...) {
                    state = BAD;
                }
                testassert(state == 3);
            } POP_POOL;
        }        
        
        testprintf("alt handler, exception thrown through\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    state++;
                    @try {
                        state++;
                        uintptr_t token = objc_addExceptionHandler(altHandler2, (void*)altHandler2);
                        // state++ inside alt handler
                        @throw [Super exception];
                        state = BAD;
                        objc_removeExceptionHandler(token);
                    } 
                    @catch (Sub *e) {
                        state = BAD;
                    }
                    state = BAD;
                } 
                @catch (id e) {
                    testassert(state == 3);
                    state++;
                    [e check];  // state++
                }
                testassert(state == 5);
            } POP_POOL;
        }
        
        
        testprintf("alt handler, nested\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    state++;
                    @try {
                        state++;
                        // same-level handlers called in FIFO order (not stack-like)
                        uintptr_t token = objc_addExceptionHandler(altHandler4, (void*)altHandler4);
                        // state++ inside alt handler
                        uintptr_t token2 = objc_addExceptionHandler(altHandler5, (void*)altHandler5);
                        // state++ inside alt handler
                        throwWithAltHandler();  // state += 2 inside
                        state = BAD;
                        objc_removeExceptionHandler(token);
                        objc_removeExceptionHandler(token2);
                    }
                    @catch (id e) {
                        testassert(state == 6);
                        state++;
                        [e check];  // state++;
                    }
                    state++;
                } 
                @catch (...) {
                    state = BAD;
                }
                testassert(state == 9);
            } POP_POOL;
        }
        
        
        testprintf("alt handler, nested, rethrows in between\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    state++;
                    @try {
                        state++;
                        // same-level handlers called in FIFO order (not stack-like)
                        uintptr_t token = objc_addExceptionHandler(altHandler5, (void*)altHandler5);
                        // state++ inside alt handler
                        uintptr_t token2 = objc_addExceptionHandler(altHandler6, (void*)altHandler6);
                        // state++ inside alt handler
                        throwWithAltHandlerAndRethrow();  // state += 3 inside
                        state = BAD;
                        objc_removeExceptionHandler(token);
                        objc_removeExceptionHandler(token2);
                    }
                    @catch (...) {
                        testassert(state == 7);
                        state++;
                        @throw;
                    }
                    state = BAD;
                } 
                @catch (id e) {
                    testassert(state == 8);
                    state++;
                    [e check];  // state++
                }
                testassert(state == 10);
            } POP_POOL;
        }
        
        
        testprintf("alt handler, exception thrown and caught inside\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    state++;
                    uintptr_t token = objc_addExceptionHandler(altHandlerFail, 0);
                    @try {
                        state++;
                        @throw [Super exception];
                        state = BAD;
                    } 
                    @catch (Super *e) {
                        state++;
                        [e check];  // state++
                    }
                    state++;
                    objc_removeExceptionHandler(token);
                } 
                @catch (...) {
                    state = BAD;
                }
                testassert(state == 5);
            } POP_POOL;
        }


#if defined(USE_FOUNDATION)
        testprintf("alt handler, rdar://10055775\n");
        
        for (i = 0; i < ALT_HANDLER_REPEAT; i++) {
            PUSH_POOL {
                state = 0;
                @try {
                    uintptr_t token = objc_addExceptionHandler(altHandler1, (void*)altHandler1);
                    {
                        id x = [NSArray array];
                        x = [NSArray array];
                    }
                    state++;
                    // state++ inside alt handler
                    [NSException raise:@"foo" format:@"bar"];
                    state = BAD;
                    objc_removeExceptionHandler(token);
                } @catch (id e) {
                    testassert(state == 2);
                }
            } POP_POOL;
        }
// defined(USE_FOUNDATION)
#endif


// alt handlers
#endif

    } POP_POOL;

#if __cplusplus  &&  __OBJC2__
    std::set_terminate(terminator);
    objc_terminate();
    fail("should not have returned from objc_terminate()");
#else
    succeed(FILENAME);
#endif
}

