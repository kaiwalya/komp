//
//  stack.cpp
//  kompute
//
//  Created by Kaiwalya Kher on 6/7/13.
//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
//

#include "stack.hpp"
#include "log.hpp"

#include <assert.h>

namespace ko {
	stack::stack(size_t szWanted, const size_t roundOff)
	{
		//Roundoff szWanted, and then add more padding so we can shift the pointer if we want
		auto allocSize = ((szWanted + roundOff - 1) & ~(roundOff - 1)) + (roundOff - 1);
		auto alloc = (uintptr_t)new unsigned char[allocSize];
		auto stackTop = alloc + allocSize;
		stackTop -= (stackTop & (roundOff - 1));
		m_pStackTop = (void*) stackTop;
		m_pStackAllocStart = (void *)alloc;
		m_szAlloc = allocSize;
		resetUsage();
		thislog("Stack size: %d, [%p %p]", (int) (unsigned int)(bytesUsed() + bytesFree()), base(), top());
	}
	
	stack::~stack()
	{
		if (m_pStackAllocStart)
		{
			checkUsage();
			thislog("%s", "");
			delete [] (unsigned char *)m_pStackAllocStart;
		}
	}
	
	
	void stack::checkUsage()
	{
		size_t unusedBytes = 0;
		auto start = (unsigned char *)m_pStackAllocStart;
		while (start < (unsigned char*)m_pStackTop && *start == 'x')
		{
			assert((uintptr_t)start < (ptrdiff_t)m_pStackAllocStart + m_szAlloc);
			unusedBytes++;
			start++;
		}
		assert(unusedBytes > (bytesUsed() + bytesFree())/4);
		if (unusedBytes <= (bytesUsed() + bytesFree())/4)
			thislog("Unused Bytes %u/%u", (unsigned int)unusedBytes, (unsigned int)(bytesUsed() + bytesFree()));
	}
	
	
}