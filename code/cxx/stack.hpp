//
//  stack.h
//  kompute
//
//  Created by Kaiwalya Kher on 6/7/13.
//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
//

#ifndef __kompute__stack__
#define __kompute__stack__


#include <cstddef>
#include <cstring>
#include <stdint.h>

namespace koro
{
	/**
	 Represents a memory block allocated on the heap which can be used as a stack for coroutines.
	 
	 */
	class stack
	{
		void * m_pStackAllocStart;
		size_t m_szAlloc;
		void * m_pStackTop;
		
		void _move(stack & s)
		{
			m_pStackAllocStart = s.m_pStackAllocStart;
			m_pStackTop = s.m_pStackTop;
			m_szAlloc = s.m_szAlloc;
			s.m_pStackTop = nullptr;
			s.m_pStackAllocStart = nullptr;
			s.m_szAlloc = 0;
		}
	public:
		stack()
		{
			m_pStackTop = nullptr;
			m_pStackAllocStart = nullptr;
			m_szAlloc = 0;
		}
		
		stack(size_t szWanted, const size_t roundOff = 16);
		
		stack(stack && s)
		{
			_move(s);
		}
		
		void swap(stack & s)
		{
			stack temp;
			temp._move(s);
			s._move(*this);
			_move(temp);
		}
		
		void checkUsage();
		bool isCurrent()
		{
			return (current() < top() && current() > base());
		}
		
		void resetUsage()
		{
			memset((void *)m_pStackAllocStart, 'x', m_szAlloc);
		}
		
		~stack();
		
		stack(const stack &) = delete;
		stack & operator = (const stack &) = delete;
		
		void * base() const {return m_pStackAllocStart;}
		void * top() const { return m_pStackTop;}
		void * current() const {
			void * ret;
			ret = &ret;
			return ret;
		}
		
		size_t bytesUsed() const
		{
			return ((uintptr_t)top() - (uintptr_t)current());
		}
		
		size_t bytesFree() const
		{
			return ((uintptr_t)current() - (uintptr_t)base());
		}
		
	};
}

#endif /* defined(__kompute__stack__) */
