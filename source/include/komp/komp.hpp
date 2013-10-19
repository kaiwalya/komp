//#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <thread>

namespace flow {
	
	using TypeIDs = uint64_t;
	using DataType = std::vector<TypeIDs>;
	using DataBusType = std::vector<DataType>;
	
	class BlockDefinition;
	class Context;
	class InterfaceProgrammer;
	class InvocationContext;
	
	using BlockHandle = void *;

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
		struct BlockInfo {
			BlockDefinition * definition;
		};
		std::map<BlockHandle, std::shared_ptr<BlockInfo>> m_blocks;
	public:
		template<typename BlockClass>
		BlockHandle createBlock() {
			std::shared_ptr<BlockInfo> bi(new BlockInfo);
			bi->definition = new BlockClass();
			m_blocks.emplace(nullptr, bi);
			return nullptr;
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
	};
};