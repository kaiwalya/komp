#if !defined kompute_hpp
#define kompute_hpp

#include "stddef.h"
#include "stdint.h"
#include "string"

namespace komp
{
	/**
	Use as base class when a delete is expected cleanup operation
	If not used, assume that no cleanup is required.
	When returned as a result of a "build" or "create" function - the caller becomes the owner.
	*/
	class virtual_destructor
	{
	public:
		virtual ~virtual_destructor();
	};
	
	template<typename T>
	class threadable: public virtual_destructor
	{
	public:
		virtual T * attachThread() = 0;
	};
	
	class thread_pool
	{
		
	};
	
	namespace mq
	{
		class ktx;
		class message;
		//class message_type;
		//class message_type_registry;
		
		/*
		class serialized_message
		{
		public:
			virtual size_t blockCount() = 0;
			virtual void * block(size_t blobCount, size_t * szOut) = 0;
			
		};
		
		class message_type
		{
		public:
			virtual serialized_message * serialize(message *) = 0;
			virtual message * deserialize(serialized_message *) = 0;
		};
		
		class message_type_registry: public threadable<message_type_registry>
		{
			
		public:
			typedef int64_t message_type_id;
			virtual message_type_id registerType(message_type * messageType) = 0;
		};
		
		enum class operation_status
		{
			success,
			end,
		};
		*/
		
		class message: virtual_destructor
		{
		public:
		};
		
		class read_window
		{
		public:
			virtual size_t size() = 0;
			virtual void resize(size_t) = 0;
			virtual void slide(size_t) = 0;
			virtual message * get(size_t messageOffset) = 0;
		};
		
		class qp_inner
		{
		public:
			virtual read_window * input();
			virtual void write(message *) = 0;
		};
		
		//Q Processor
		class qp_logic: virtual_destructor
		{
			virtual void process(qp_inner *) = 0;
		};
		
		class qp_outer: virtual_destructor
		{
		public:
			virtual read_window * output();
		};
				
		class ktx: public threadable<ktx>
		{
		public:
			static ktx * create();
			virtual qp_outer * createQProcessor(qp_logic *) = 0;
		};
	}
}

#endif