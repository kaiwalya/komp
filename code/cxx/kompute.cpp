#include "kompute.hpp"
#include "komp.hpp"
#include "log.hpp"

namespace komp
{

	namespace mq
	{
		ctx_shared::ctx_shared()
		{
			m_nWorkers = 0;
			m_nBootedWorkers = 0;
			m_nShutdownWorkers = 0;
			m_workersShouldShutdown = 0;
			thisCheckpoint();
			bootPool();
		}
		
		ctx_shared::~ctx_shared()
		{
			shutdownPool();
			thisCheckpoint();
		}
		void ctx_shared::shutdownPool()
		{
			auto l = tsharedcontext_lock();
			auto lShutdown = lock(m_workerShutdownMutex);
			m_workersShouldShutdown = 1;
			while(m_nShutdownWorkers != m_nWorkers)
			{
				m_workerShuttingdownCondition.wait(lShutdown);
			}
			m_nShutdownWorkers = 0;
			m_workerShutdownCondition.notify_all();
		}
		
		void ctx_shared::bootPool()
		{
			auto l = tsharedcontext_lock();
			auto lInfo = lock(m_workerInfoLock);
			auto lBoot = lock(m_workerBootMutex);
			if(!m_pool)
			{
				std::unique_ptr<thread::pool> p(new thread::pool);
				m_pool.swap(p);
				
				m_pool->start([this, &lBoot](){
					auto params = new worker_params();
					params->m_workerID = m_nWorkers++;
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
			thislog("Worked (%d) Booting", (int)params->m_workerID);
			//Boot phase
			{
				auto lBoot = lock(m_workerBootMutex);
				m_nBootedWorkers++;
				m_workerBootingCondition.notify_one();
				thislog("Worked (%d) Booted", (int)params->m_workerID);
				while(m_nBootedWorkers)
					m_workerBootedCondition.wait(lBoot);
//				std::chrono::milliseconds ms(1000);
//				std::this_thread::sleep_for(ms);
			}
			thislog("Worked (%d) Started", (int)params->m_workerID);
			
			{
				ctx local(this);

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
		
		ctx::ctx(ctx_shared * shared)
		:thread::localized<ctx_shared, ctx>(shared)
		,m_io(*this)
		{
			thisCheckpoint();
		}
		
		ctx::~ctx()
		{
			thisCheckpoint();
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
			ctx c(new ctx_shared);
			auto io = c.dispatch([](ctx & cin){
				auto io = cin.current();
			});
		}
	};
};