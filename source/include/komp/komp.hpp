//#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <list>
#include <typeinfo>
#include "typ.hpp"
#include <tutil.hpp>

namespace komp {
	
	class BlockDefinition;
	class Context;
	class InterfaceProgrammer;
	class InvocationContext;
	
	enum BlockType {
		NoSideEffect,
		InternalState,
		ExternalState,
	};
	
	class InterfaceProgrammer {
	protected:
		virtual void setInputInterface(const typ::TypeInfo & info) = 0;
		virtual void setOutputInterface(const typ::TypeInfo & info) = 0;
		typ::TypeRegistry & m_reg;
	public:
		InterfaceProgrammer(typ::TypeRegistry & reg):m_reg(reg){}
		
		template<typename TBus>
		void setInputInterface() {
			setInputInterface(m_reg.findTypeInfo<TBus>());
		}
		
		template<typename TBus>
		void setOutputInterface() {
			setOutputInterface(m_reg.findTypeInfo<TBus>());
		}
		
		virtual void setBlockType(BlockType type) = 0;
		virtual ~InterfaceProgrammer() {}
	};
	
	
	//Allows block instance to get data it is getting called with
	//Input bus and output bus
	//as well as any internal state.
	class InvocationContext {
	protected:
	public:
		//Internal State
		//getInernalState
		
		template<typename Type>
		auto getInput(size_t index) -> void *{

		}
		
		template<typename Type>
		auto getOutput(size_t index) -> void *{
			return nullptr;
		}
	};
	
	class Context {

		using Mutex = std::mutex;
		using Condition = std::condition_variable;
		using Lock = std::unique_lock<Mutex>;

		enum class BlockState {
			Initialize, Running, Finalize,
		};
		struct BlockInfo {
			BlockDefinition * definition;
			BlockType type;
			const typ::TypeInfo * inputInfo;
			const typ::TypeInfo * outputInfo;
		};
		
		using BlockInfoPtr = BlockInfo *;
		using BlockInfoPtrList = std::list<BlockInfoPtr>;
		using BlockInfoIter = BlockInfoPtrList::iterator;
		
		struct BlockListInfo {
			BlockInfoPtrList m_list;
			Mutex m_mutex;
			BlockListInfo(){}
			//BlockListInfo(const BlockListInfo &) {assert(false);};
			Lock lock() {
				return Lock(m_mutex);
			}
			BlockInfoPtrList & list() {
				return m_list;
			}
		};
		
		
		class InterfaceProgrammerImpl:public InterfaceProgrammer {
			BlockInfo & info;
		public:
			InterfaceProgrammerImpl(typ::TypeRegistry & reg, BlockInfo & info):InterfaceProgrammer(reg), info(info) {
			}
			virtual void setBlockType(BlockType type) {
				this->info.type = type;
			}
			virtual void setInputInterface(const typ::TypeInfo & info) {
				this->info.inputInfo = &info;
			}
			virtual void setOutputInterface(const typ::TypeInfo & info) {
				this->info.outputInfo = &info;
			}
		};

		
		komp::thread::pool m_pool;
		std::map<BlockState, std::shared_ptr<BlockListInfo>> m_blocks;
		
		BlockListInfo & findListInfo(BlockState forState) {
			return *(m_blocks[forState].get());
		}
		void initialize(BlockInfoIter it);
		void run(BlockInfoIter it);
		void finalize(BlockInfoIter it);
				
		Mutex m_mBlockCount;
		Condition m_cBlockCount;
		size_t m_nBlockCountTotal;
		size_t m_nBlockCountCurrent;
		
		void blockCountIncr() {
			Lock lock(m_mBlockCount);
			m_nBlockCountCurrent++;
			m_nBlockCountTotal++;
		}
		void blockCountDecr() {
			Lock lock(m_mBlockCount);
			m_nBlockCountCurrent--;
			m_cBlockCount.notify_one();
		}
		void blockCountZero() {
			Lock lock(m_mBlockCount);
			while(m_nBlockCountCurrent) {
				m_cBlockCount.wait(lock);
			}
		}
		
		typ::TypeRegistry m_typeRegistry;
	public:
		using BlockHandle = BlockInfoIter;
		Context();
		~Context();
		template<typename BlockClass>
		BlockHandle createBlock() {
			blockCountIncr();
			
			auto bi = new BlockInfo;
			bi->definition = new BlockClass();
			auto & li = findListInfo(BlockState::Initialize);
			auto lock = li.lock();
			auto & list = li.list();
			auto it = list.insert(list.begin(), bi);
			assert(list.size());
			m_pool.addJob([this, it](){
				initialize(it);
			});
			return it;
		}
		void join(BlockHandle inputOfThis, BlockHandle outputOfThis) {}
	};
	
	
	//All exposed blocks need to implement this class;
	class BlockDefinition{
	public:
		virtual void defineInterface(InterfaceProgrammer & programmer) const = 0;
		virtual void performDefinition(InvocationContext & ctx) = 0;
		virtual ~BlockDefinition(){}
	};
};
