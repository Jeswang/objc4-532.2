// TEST_CONFIG

#include "test.h"
#include "testroot.i"
#include <objc/runtime.h>

static int state;

#define ONE 1
#define TWO 2
#define LENGTH 3
#define COUNT 4

@interface Super : TestRoot @end
@implementation Super
+(void) one { state = ONE; }
+(void) two { state = TWO; }
+(void) length { state = LENGTH; }
+(void) count { state = COUNT; }
@end

#define checkExchange(s1, v1, s2, v2)                                   \
    do {                                                                \
        Method m1, m2;                                                  \
                                                                        \
        /* Check unexchanged version */                                 \
        state = 0;                                                      \
        [Super s1];                                                     \
        testassert(state == v1);                                        \
        state = 0;                                                      \
        [Super s2];                                                     \
        testassert(state == v2);                                        \
                                                                        \
        /* Exchange */                                                  \
        m1 = class_getClassMethod([Super class], @selector(s1));        \
        m2 = class_getClassMethod([Super class], @selector(s2));        \
        testassert(m1);                                                 \
        testassert(m2);                                                 \
        method_exchangeImplementations(m1, m2);                         \
                                                                        \
        /* Check exchanged version */                                   \
        state = 0;                                                      \
        [Super s1];                                                     \
        testassert(state == v2);                                        \
        state = 0;                                                      \
        [Super s2];                                                     \
        testassert(state == v1);                                        \
                                                                        \
        /* NULL should do nothing */                                    \
        method_exchangeImplementations(m1, NULL);                       \
        method_exchangeImplementations(NULL, m2);                       \
        method_exchangeImplementations(NULL, NULL);                     \
                                                                        \
        /* Make sure NULL did nothing */                                \
        state = 0;                                                      \
        [Super s1];                                                     \
        testassert(state == v2);                                        \
        state = 0;                                                      \
        [Super s2];                                                     \
        testassert(state == v1);                                        \
                                                                        \
        /* Put them back */                                             \
        method_exchangeImplementations(m1, m2);                         \
                                                                        \
        /* Check restored version */                                    \
        state = 0;                                                      \
        [Super s1];                                                     \
        testassert(state == v1);                                        \
        state = 0;                                                      \
        [Super s2];                                                     \
        testassert(state == v2);                                        \
    } while (0) 

int main()
{
    // Check ordinary selectors
    checkExchange(one, ONE, two, TWO);

    // Check vtable selectors
    checkExchange(length, LENGTH, count, COUNT);

    // Check ordinary<->vtable and vtable<->ordinary
    checkExchange(count, COUNT, one, ONE);
    checkExchange(two, TWO, length, LENGTH);

    succeed(__FILE__);
}
