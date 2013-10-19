//
//  log.h
//  kompute
//
//  Created by Kaiwalya Kher on 6/7/13.
//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
//

#ifndef kompute_log_h
#define kompute_log_h

/********
 *Logging
 **////
#include <typeinfo>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_LEVEL 0
#if defined(LOG_LEVEL) && LOG_LEVEL > 0
#define LOG_ENABLED 1
#define thislog(fmt, ...) {\
auto __classname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, nullptr);\
printf("[%s(%p)::%s()] " fmt "\n", __classname, this, __FUNCTION__, __VA_ARGS__);\
free(__classname);\
}
#define funclog(fmt, ...) printf("[%s, %d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)
#define thisCheckpoint() thislog("%s", "")
#else
#define LOG_ENABLED 0
#define thislog(fmt, ...) ((void)0)
#define funclog(fmt, ...) ((void)0)
#define thisCheckpoint() ((void)0)
#endif


#endif
