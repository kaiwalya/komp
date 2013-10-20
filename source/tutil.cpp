#include <mutex>


#include "tutil.hpp"
#include "log.hpp"



namespace komp
{
	namespace thread
	{
		
		void pool::start()
		{
			m_nWorkersEntered = 0;
			m_nWorkersExited = 0;
			auto nCores = std::thread::hardware_concurrency();
			if (nCores == 0)
			{
				nCores = 16;
			}
			
			auto worker = [this](){
				{
					Lock lock(m_mWorkerEnter);
					m_nWorkersEntered--;
					//thislog("Entered, %d", (int)m_nWorkersEntered);
					m_cWorkerEnter.notify_one();
				}
				while(1)
				{
					std::function<void(void)> job;
					{
						Lock lock(m_mWorkerSchedule);
						while (!m_bQuit && !m_jobs.size()) {
							m_cWorkerSchedule.wait(lock);
						}
						if (m_bQuit) {
							break;
						}
						job = m_jobs.back();
						m_jobs.pop_back();
					}
					job();
				}
				
				{
					Lock lock(m_mWorkerExit);
					m_nWorkersExited++;
					//thislog("Exit, %d", (int)m_nWorkersExited);
					m_cWorkerExit.notify_one();
				}
			};

			{
				std::unique_lock<std::mutex> lock(m_mWorkerEnter);
				while (m_nWorkersEntered < nCores) {
					m_pool.emplace_back(std::thread(worker));
					m_nWorkersEntered++;
					//thislog("Entering, %d", (int)m_nWorkersEntered);
				}
				while(m_nWorkersEntered) {
					m_cWorkerEnter.wait(lock);
				}
				m_nWorkersEntered = nCores;
			}
		}
		
		pool::pool()
		{
			start();
		}
		
		
		pool::~pool()
		{

			{
				Lock lock2(m_mWorkerSchedule);
				m_bQuit = true;
				m_cWorkerSchedule.notify_all();
			}
			for(auto & thread: m_pool)
			{
				if (thread.joinable())
					thread.join();
			}
			{
				Lock lock(m_mWorkerExit);
				while(m_nWorkersExited != m_nWorkersEntered) {
					m_cWorkerExit.wait(lock);
				}
			}
			
		}
		
		class test_localize_local;

		
		class test_localize_shared: public localizable<test_localize_shared, test_localize_local>
		{
		public:
			test_localize_shared()
			{
				thisCheckpoint();
			}
			
			~test_localize_shared()
			{
				thisCheckpoint();
			}
		};
		
		class test_localize_local: public localized<test_localize_shared, test_localize_local>
		{
			
		public:
			test_localize_local(test_localize_shared * shared)
			:localized<komp::thread::test_localize_shared, komp::thread::test_localize_local>(shared)
			{
				tsharedcontext()->tlocalcontext_attach(this);
				thisCheckpoint();
			}
			
			~test_localize_local()
			{
				tsharedcontext()->tlocalcontext_detach(this);
				thisCheckpoint();
			}
		};
		
		void test()
		{
			auto local = new test_localize_local(new test_localize_shared());
			
			assert(local == local->tsharedcontext()->tlocalcontext_find());
			
			std::thread t([local](){
				std::unique_ptr<test_localize_local> l(new test_localize_local(local->tsharedcontext()));
				assert(l.get() != local);
				assert(l.get() == local->tsharedcontext()->tlocalcontext_find());
			});
			t.join();
			delete local;

		}
	}
}
