#include "kompute.hpp"
#include "komp.hpp"
#include "log.hpp"

namespace komp
{

	namespace mq
	{
		ctx_shared::ctx_shared()
		{
			m_nExternalCtx = 0;
			m_nWorkers = 0;
			m_nBootedWorkers = 0;
			m_nShutdownWorkers = 0;
			thisCheckpoint();
			//bootPool();
		}
		
		ctx_shared::~ctx_shared()
		{
			thisCheckpoint();
		}
		
		void ctx_shared::tlocalcontext_attach_locked(ctx * local)
		{
			thread::localizable<ctx_shared, ctx>::tlocalcontext_attach_locked(local);
			if(local->role() == ctx_role::external)
			{
				if (!m_nExternalCtx)
				{
					//thislog("%s", "First external contexts, booting pool");
					bootPool();
				}
				m_nExternalCtx++;
				//thislog("%d external contexts+", (int)m_nExternalCtx);
			}
		}
		
		void ctx_shared::tlocalcontext_detach_locked(ctx * local)
		{
			if(local->role() == ctx_role::external)
			{
				m_nExternalCtx--;
				//thislog("%d external contexts-", (int)m_nExternalCtx);
				//If the last of the external contexts is going away, we trigger a shutdown here.
				if (!m_nExternalCtx)
				{
					//thislog("%s", "Last external contexts, shutting pool");
					triggerPoolShutdown();
					waitForPoolShutdown();
				}
			}
			thread::localizable<ctx_shared, ctx>::tlocalcontext_detach_locked(local);
		}

		void ctx_shared::triggerPoolShutdown()
		{
			//This function should always be called from tlocalcontext_detach_locked,
			//so we dont take the lock
			//auto l = tsharedcontext_lock();
			for(auto &it : m_workerinfo)
			{
				auto worker_lock = lock(it.second->m_workerMutex);
				it.second->m_quit = true;
				it.second->m_workerCondition.notify_one();
			}
			//thislog("%s", "Shutdown triggered");
		}
		
		void ctx_shared::waitForPoolShutdown()
		{
			auto lShutdown = lock(m_workerShutdownMutex);
			//thislog("%s", "Waiting for pool shutdown");
			while(m_nShutdownWorkers != m_nWorkers)
			{
				//thislog("%s, %d left", "Wait for pool shutdown", (int)(m_nWorkers - m_nShutdownWorkers));
				m_workerShuttingdownCondition.wait(lShutdown);
			}
			m_nShutdownWorkers = 0;
			m_workerShutdownCondition.notify_all();
		}
		
		void ctx_shared::bootPool()
		{
			//This function should always be called from tlocalcontext_detach_locked,
			//so we dont take the lock
			//auto l = tsharedcontext_lock();
			auto lInfo = lock(m_workerInfoLock);
			auto lBoot = lock(m_workerBootMutex);
			if(!m_pool)
			{
				std::unique_ptr<thread::pool> p(new thread::pool);
				m_pool.swap(p);
				
				m_pool->start([this, &lBoot](){
					auto params = new worker_params();
					{
						auto worker_lock = lock(params->m_workerMutex);
						params->m_workerID = m_nWorkers++;
						params->m_quit = false;
					}
					m_workerinfo.insert({params->m_workerID, params});
					return [this, params](){
						this->worker(params);
					};
					
				});
				
				while(m_nWorkers != m_nBootedWorkers)
				{
					m_workerBootingCondition.wait(lBoot);
				}
				m_nBootedWorkers = 0;
				thislog("Worker Count: %d", (int)workerCount());
				m_workerBootedCondition.notify_all();
			}
		}
		
		
		void ctx_shared::worker(worker_params * params)
		{
			thislog("Worker (%d) Booting", (int)params->m_workerID);
			//Boot phase
			{
				auto lBoot = lock(m_workerBootMutex);
				m_nBootedWorkers++;
				m_workerBootingCondition.notify_one();
				thislog("Worker (%d) Booted", (int)params->m_workerID);
				while(m_nBootedWorkers)
					m_workerBootedCondition.wait(lBoot);
//				std::chrono::milliseconds ms(1000);
//				std::this_thread::sleep_for(ms);
			}
			thislog("Worker (%d) Started", (int)params->m_workerID);
			
			{
				auto worker_lock = lock(params->m_workerMutex);
				while(!params->m_quit)
				{
					//thislog("Worker (%d) waiting for work", (int)params->m_workerID);
					params->m_workerCondition.wait(worker_lock);
					//thislog("Worker (%d) woken up", (int)params->m_workerID);
				}
			}
			

			//Shutdown phase
			{
				size_t id = params->m_workerID;
				thislog("Worker (%d) Stopping", (int)id);
				auto lShutdown = lock(m_workerShutdownMutex);
				{
					auto lInfo = lock(m_workerInfoLock);
					m_workerinfo.erase(params->m_workerID);
				}
				delete params;
				m_nShutdownWorkers++;
				m_workerShuttingdownCondition.notify_one();
				thislog("Worker (%d) Shutting down", (int)id);
				while(m_nShutdownWorkers)
					m_workerShutdownCondition.wait(lShutdown);
				thislog("Worker (%d) Shut down", (int)id);
				
			}
		}
		
		processor * ctx_shared::dispatch(std::function<void (ctx &)> &f)
		{
			auto proc =  new processor(f);
			delete proc;
			return nullptr;
		}
		
		ctx::ctx(ctx_shared * shared, ctx_role r)
		:thread::localized<ctx_shared, ctx>(shared)
		,m_role(r)
		,m_io(*this)
		{
			thisCheckpoint();
			tsharedcontext()->tlocalcontext_attach(this);
		}
		
		ctx::~ctx()
		{
			thisCheckpoint();
			tsharedcontext()->tlocalcontext_detach(this);
		}
		
		ctx_role ctx::role() const
		{
			return m_role;
		}
		
		io ctx::dispatch(std::function<void (ctx &)> &f)
		{
			tsharedcontext()->dispatch(f);
			return io(*this);
		}
		
		io ctx::dispatch(std::function<void (ctx &)> &&f)
		{
			return dispatch(f);
		}
		
		io & ctx::current()
		{
			return m_io;
		}
		
		io::io(ctx & c):m_ctx(c)
		{
		}
		
		io::~io()
		{
			
		}
		
		io & io::operator=(const komp::mq::io &other)
		{
			assert(false);
			return *this;
		}
		
		void test()
		{
			{
				ctx c(new ctx_shared);
				auto io = c.dispatch([](ctx & cin){
					auto io = cin.current();
				});
			}
			funclog("%s", "TestFinished");
		}
	};
};