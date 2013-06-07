//
//  ko.cpp
//  Code
//
//  Created by Kaiwalya Kher on 6/7/13.
//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
//

#include "ko.hpp"
#include "log.hpp"


/////////
//def ctx
//
namespace ko
{
	ctx::ctx()
	{
		thisCheckpoint();
		createCurrentThreadContext();
	}
	
	ctx::~ctx()
	{
		thisCheckpoint();
	}
	
	std::thread::id ctx::getCurrentThreadID()
	{
		return ctx_thread::getCurrentThreadID();
	}
	
	ctx_thread * ctx::getCurrentThreadContext()
	{
		auto id = getCurrentThreadID();
		assert(m_threadContexts.find(id) != m_threadContexts.end());
		return m_threadContexts[id];
	}
	
	ctx_thread * ctx::createCurrentThreadContext()
	{
		auto id = getCurrentThreadID();
		assert(m_threadContexts.find(id) == m_threadContexts.end());
		auto ret = new ctx_thread(this, id);
		m_threadContexts.emplace(std::make_pair(id, ret));
		return ret;
	}
	
	ctx_thread * ctx::detachCurrentThreadContext()
	{
		auto ret = getCurrentThreadContext();
		assert(ret);
		m_threadContexts.erase(getCurrentThreadID());
		if (!m_threadContexts.size())
		{
			delete this;
		}
		return ret;
	}
}

/////////
//def ctx_thread
//
namespace ko
{
	ctx_thread::ctx_thread(ctx * ctx, std::thread::id tid): m_ctx(ctx), m_threadID(tid)
	{
		assert(m_threadID == std::this_thread::get_id());
		thisCheckpoint();
	}
	
	ctx_thread::~ctx_thread()
	{
		thisCheckpoint();
		assert(this == m_ctx->getCurrentThreadContext());
		m_ctx->detachCurrentThreadContext();
	}
	
	std::thread::id ctx_thread::getCurrentThreadID()
	{
		return std::this_thread::get_id();
	}
	
	ktx * ctx_thread::attachThread()
	{
		return m_ctx->createCurrentThreadContext();
	}
}
