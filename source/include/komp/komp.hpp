//#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <list>
#include <typeinfo>

#include <tutil.hpp>

namespace komp {
	
	using TypeIDs = uint64_t;
	using DataType = std::vector<TypeIDs>;
	using DataBusType = std::vector<DataType>;
	
	class BlockDefinition;
	class Context;
	class InterfaceProgrammer;
	class InvocationContext;
	
	enum BlockType {
		NoSideEffect,
		InternalState,
		ExternalState,
	};
	
	namespace native {
		using Boolean = bool *;
		using Integer32 = int32_t *;
		using Size = size_t *;
		using Char = char *;
		
		template<typename Type, Size DimensionCount>
		struct Array {
			Size size[DimensionCount];
			const Type data() const;
			Type data();
		};
		
		struct StreamGeneric {
			//Window also implies that stream migh end with incomplete data
			//Take that into account when designing this API
			void setWindow(Size sz) {}
			
			//Inititalizes the stream, gets more data to send receive;
			bool next() {return false;}
			
			//Will send EOF to following stream
			void close() {}
			
			void * genericData() {return nullptr;}
		};
		
		template<typename Type>
		struct Stream : StreamGeneric{
			
			
			//Readable Streams will return null when
			//stream has ended but there is not enough
			//data to full the window
			Type data() {return *(Type *)genericData();}
			
		};
	}
	
	class Context {

		using Mutex = std::mutex;
		using Condition = std::condition_variable;
		using Lock = std::unique_lock<Mutex>;
		
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
		
		enum class BlockState {
			Initialize, Running, Finalize,
		};
		struct BlockInfo {
			BlockDefinition * definition;
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
		
		
		komp::thread::pool m_pool;
		std::map<BlockState, std::shared_ptr<BlockListInfo>> m_blocks;
		
		BlockListInfo & findListInfo(BlockState forState) {
			return *(m_blocks[forState].get());
		}
		void initialize(BlockInfoIter it);
		void run(BlockInfoIter it);
		void finalize(BlockInfoIter it);
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
	
	class InterfaceProgrammer {
	public:
		template<typename TArray>
		void setInputInterface() {}
		
		template<typename TArray>
		void setOutputInterface() {}
		
		void setBlockType(BlockType type) {};
	};
	
	//Allows block instance to get data it is getting called with
	//Input bus and output bus
	//as well as any internal state.
	class InvocationContext {
	protected:
		void * genericGetInput(native::Size) {
			return nullptr;
		}
		
		void * genericGetOutput(native::Size) {
			return nullptr;
		}
	public:
		//Internal State
		//getInernalState
		
		template<typename Type>
		Type & getInput(native::Size index) {
			return *(Type *)genericGetInput(index);
		}
		
		template<typename Type>
		Type & getOutput(native::Size index) {
			return *(Type *)genericGetOutput(index);
		}
	};
	
	//All exposed blocks need to implement this class;
	class BlockDefinition{
	public:
		virtual void defineInterface(InterfaceProgrammer & programmer) const = 0;
		virtual void performDefinition(InvocationContext & ctx) = 0;
		virtual ~BlockDefinition(){}
	};
};