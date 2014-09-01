//
//  main.m
//  debug-objc
//
//  Created by jeswang on 9/1/14.
//
//

#import <Foundation/Foundation.h>
#import <objc/objc.h>
#import <objc/runtime.h>

void ReportFunction(id self, SEL _cmd)
{
    NSLog(@" >> This object is %p.", self);
    NSLog(@" >> Class is %@, and super is %@.", [self class], [self superclass]);
    
    Class prevClass = NULL;
    int count = 1;
    for (Class currentClass = [self class]; currentClass; ++count)
    {
        prevClass = currentClass;
        
        NSLog(@" >> Following the isa pointer %d times gives %p", count, currentClass);
        
        currentClass = object_getClass(currentClass);
        if (prevClass == currentClass)
            break;
    }
    
    NSLog(@" >> NSObject's class is %p", [NSObject class]);
    NSLog(@" >> NSObject's meta class is %p", object_getClass([NSObject class]));
}

int main (int argc, const char * argv[])
{
    
    @autoreleasepool
    {
        Class newClass = objc_allocateClassPair([NSString class], "NSStringSubclass", 0);
        class_addMethod(newClass, @selector(report), (IMP)ReportFunction, "v@:");
        objc_registerClassPair(newClass);
        
        id instanceOfNewClass = [[newClass alloc] init];
        [instanceOfNewClass performSelector:@selector(report)];
    }
    
    return 0;
}
