//#include <string>
#include <vector>

namespace flow {
	
	using TypeIDs = uint64_t;
	using DataType = std::vector<TypeIDs>;
	using DataBusType = std::vector<DataType>;
	
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
		
		template<typename Type>
		struct Stream {
			//Window also implies that stream migh end with incomplete data
			//Take that into account when designing this API
			void setWindow(Size sz);
			
			//Inititalizes the stream, gets more data to send receive;
			bool next();
			
			//Readable Streams will return null when
			//stream has ended but there is not enough
			//data to full the window
			Type data();
			
			//Will send EOF to following stream
			void close();
		};
	}
	
	enum BlockType {
		NoSideEffect,
		InternalState,
		ExternalState,
	};
	
	using BlockHandle = void *;
	class Context {
	public:
		template<typename BlockClass>
		BlockHandle createBlock();
		void join(BlockHandle inputOfThis, BlockHandle outputOfThis);
	};
	
	class InterfaceProgrammer {
	public:
		template<typename TArray>
		void setInputInterface();
		
		template<typename TArray>
		void setOutputInterface();
		
		void setBlockType(BlockType type);
	};
	
	//Allows block instance to get data it is getting called with
	//Input bus and output bus
	//as well as any internal state.
	class InvocationContext {
	public:
		//Internal State
		//getInernalState
		
		template<typename Type>
		Type & getInput(native::Size index);
		
		template<typename Type>
		Type & getOutput(native::Size index);
	};
	
	//All exposed blocks need to implement this class;
	class BlockDeclaration{
	public:
		virtual void defineInterface(InterfaceProgrammer & programmer) const;
		virtual void performDefinition(InvocationContext & ctx) = 0;
	};
};