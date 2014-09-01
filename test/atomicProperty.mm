// TEST_CONFIG CC=clang

#include "test.h"
#include <objc/runtime.h>
#include <objc/objc-internal.h>
#import <Foundation/NSObject.h>

class SerialNumber {
    size_t _number;
public:
    SerialNumber() : _number(42) {}
    SerialNumber(const SerialNumber &number) : _number(number._number + 1) {}
    SerialNumber &operator=(const SerialNumber &number) { _number = number._number + 1; return *this; }

    int operator==(const SerialNumber &number) { return _number == number._number; }
    int operator!=(const SerialNumber &number) { return _number != number._number; }
};

@interface TestAtomicProperty : NSObject {
    SerialNumber number;
}
@property(atomic) SerialNumber number;
@end

@implementation TestAtomicProperty

#if 1 // with new enough compiler, this will be synthesized automatically.

extern void objc_copyCppObjectAtomic(void *dest, const void *src, void (*copyHelper) (void *dest, const void *source));

static void copySerialNumber(void *d, const void *s) {
    SerialNumber *dest = (SerialNumber *)d;
    const SerialNumber *src = (const SerialNumber *)s;
    dest->operator=(*src);
}

- (SerialNumber)number {
    SerialNumber result;
    objc_copyCppObjectAtomic(&result, &number, copySerialNumber);
    return result;
}

- (void)setNumber:(SerialNumber)aNumber {
    objc_copyCppObjectAtomic(&number, &aNumber, copySerialNumber);
}

+(void)initialize {
    testwarn("rdar://6137845 compiler should synthesize calls to objc_copyCppObjectAtomic");
}

#else
@synthesize number;
#endif    

@end

int main()
{
    PUSH_POOL {
        SerialNumber number;
        TestAtomicProperty *test = [TestAtomicProperty new];
        test.number = number;
        testassert(test.number != number);
    } POP_POOL;

    succeed(__FILE__);
}
