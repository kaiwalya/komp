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
			m_nJobsScheduledCurrent = 0;
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
						//Quit only if there are no jobs executing or pending and we got external quit signal
						while (!(shouldQuit()) && !m_jobs.size()) {
							m_cWorkerJobLess.notify_one();
							m_cWorkerSchedule.wait(lock);
						}
						if (shouldQuit()) {
							break;
						}
						assert(m_jobs.size());
						job = m_jobs.back();
						m_nJobsScheduledCurrent++;
						m_nJobsScheduledTotal++;
						m_jobs.pop_back();
					}
					
					job();
					
					{
						Lock lock(m_mWorkerSchedule);
						m_nJobsScheduledCurrent--;
					}
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
		
		void pool::stop() {
			{
				Lock lock(m_mWorkerSchedule);
				while (!canQuit()) {
					m_cWorkerJobLess.wait(lock);
				}
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
		
		pool::pool():m_nJobsScheduledCurrent(0), m_nJobsScheduledTotal(0), m_bQuit(false)
		{
			start();
		}
		
		
		pool::~pool()
		{
			stop();
			//thislog("m_nJobsScheduledTotal: %d", (int)m_nJobsScheduledTotal)
		}
		
		void pool::addJob(std::function<void ()> && job) {
			addJob(job);
		}
		
		void pool::addJob(std::function<void ()> & job) {
			{
				Lock lock(m_mWorkerSchedule);
				m_jobs.insert(m_jobs.begin(), job);
				m_cWorkerSchedule.notify_one();
				assert(m_jobs.size());
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
