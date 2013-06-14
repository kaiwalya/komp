#if !defined THREAD_UTILS_HPP
#define THREAD_UTILS_HPP
#include <memory>
#include <thread>
#include <map>
#include <assert.h>
#include <vector>

namespace komp
{
	namespace thread
	{
		class pool
		{
			std::vector<std::thread> m_pool;
		public:
			pool(){}
			void start(std::function<std::function<void(void)>(void)> & f);
			void start(std::function<std::function<void(void)>(void)> && f);
			~pool();
		};
		
		template<typename tshared, typename tlocal>
		class localized
		{
		private:
			tshared * m_shared;
		public:
			localized(tshared * shared): m_shared(shared)
			{
				shared->tlocalcontext_attach(static_cast<tlocal*>(this));
			}
			~localized()
			{
				m_shared->tlocalcontext_detach(static_cast<tlocal*>(this));
			}
			
			tshared * tsharedcontext()
			{
				return m_shared;
			}
		};
		
		template<typename tshared, typename tlocal>
		class localizable
		{
		private:
			typedef localizable<tshared, tlocal> mytype;
			typedef std::map<std::thread::id, tlocal *> tlocal_map;
			tlocal_map m_localmap;
			std::mutex m_localmap_mutex;
			tshared * m_shared;
			
			
		private:
			tlocal * tlocalcontext_create()
			{
				return new tlocal(m_shared);
			}
		public:
			typedef std::mutex mutex_type;
			typedef std::unique_lock<mutex_type> lock_type;
			mutex_type & tsharedcontext_mutex()
			{
				return m_localmap_mutex;
			}
			
			lock_type tsharedcontext_lock()
			{
				return lock_type(tsharedcontext_mutex());
			}
			
			localizable()
			{
				m_shared = static_cast<tshared *>(this);
			}
			
			virtual ~localizable()
			{
			}
			
			tlocal * tlocalcontext_find()
			{
				auto tid = std::this_thread::get_id();
				auto lock = tsharedcontext_lock();
				if (m_localmap.find(tid) == m_localmap.end())
					return nullptr;
				return m_localmap[tid];
			}
			
			void tlocalcontext_attach(tlocal * local)
			{
				auto tid = std::this_thread::get_id();
				auto lock = tsharedcontext_lock();
				assert(m_localmap.find(tid) == m_localmap.end());
				m_localmap.emplace(std::make_pair(tid, local));
			}
			
			/*
			tlocal * tlocalcontext_find_or_make()
			{
				auto tid = std::this_thread::get_id();
				std::unique_lock<decltype(m_localmap_mutex)> locl(m_localmap_mutex);
				if (m_localmap.find(tid) == m_localmap.end())
					return locked_create_and_add_to_map(tid);
				return m_localmap[tid];
			}
			*/
			
			void tlocalcontext_detach(tlocal * local)
			{
				bool bDelete = false;
				auto tid = std::this_thread::get_id();
				{
					auto lock = tsharedcontext_lock();
					assert(m_localmap.find(tid) != m_localmap.end());
					assert(m_localmap[tid] == local);
					m_localmap.erase(tid);
					if(!m_localmap.size()) bDelete = true;
				}
				if (bDelete) delete this;
			}
		};
		
		void test();
	}
};

#endif