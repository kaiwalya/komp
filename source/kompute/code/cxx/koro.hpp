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
namespace koro
{
	class ctx;
	class ctx_thread;
	
	
	class ctx
	{
	private:
		typedef std::map<std::thread::id, ctx_thread *> ThreadContextMap;
		std::mutex m_mThreadContext;
		ThreadContextMap m_threadContexts;
	public:
		ctx();
		virtual ~ctx();
		
		ctx_thread * getThreadContext(std::thread::id threadID);
		void attachThreadContext(std::thread::id threadID, ctx_thread * ctxThread);
		void detachThreadContext(std::thread::id threadID);
		
	};
	
	class ctx_thread: public ktx
	{
	private:
		ctx * m_ctx;
		std::thread::id m_threadID;
	public:
		static std::thread::id getCurrentThreadID();
	public:
		ctx_thread();
		ctx_thread(ctx * ctx);
		virtual ~ctx_thread();
		ktx * attachThread() override;
	};
}

#endif /* defined(__Code__ko__) */
