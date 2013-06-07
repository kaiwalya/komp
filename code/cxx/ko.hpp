//
//  ko.h
//  Code
//
//  Created by Kaiwalya Kher on 6/7/13.
//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
//

#ifndef __Code__ko__
#define __Code__ko__

#include "koroutine.hpp"
#include <thread>
#include <map>
/////////
//decl ctx
//
namespace ko
{
	class ctx;
	class ctx_thread;
	
	
	class ctx
	{
	private:
		typedef std::map<std::thread::id, ctx_thread *> ThreadContextMap;
		ThreadContextMap m_threadContexts;
	public:
		ctx();
		virtual ~ctx();
		static std::thread::id getCurrentThreadID();
		ctx_thread * getCurrentThreadContext();
		ctx_thread * createCurrentThreadContext();
		ctx_thread * detachCurrentThreadContext();
	};
	
	class ctx_thread: public ktx
	{
	private:
		ctx * m_ctx;
		std::thread::id m_threadID;
	public:
		static std::thread::id getCurrentThreadID();
	public:
		ctx_thread(ctx * ctx, std::thread::id);
		virtual ~ctx_thread();
		ktx * attachThread() override;
	};
}

#endif /* defined(__Code__ko__) */
