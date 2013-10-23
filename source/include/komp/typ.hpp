#if !defined(KOMP_TYP_HPP_)
#define KOMP_TYP_HPP_

#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <type_traits>

namespace komp {
	namespace type {

		template<typename TNative>
		struct Void;
		
		template<typename TNative>
		struct Character;
		
		template<typename TNative>
		struct Integer32;
		
		template<typename TNative, typename TInner>
		struct Array;
		
		template<typename TNative, typename TInner>
		struct Stream;
		
		template<typename  Next>
		struct ChainSelectForwarder {
			using Type = typename Next::Type;
		};
		
		template<> struct ChainSelectForwarder<void> {
			using Type = void;
		};
		
		template<typename T, typename Next = void>
		struct ChainSelectBasic: ChainSelectForwarder<Next> {};
		
		template<typename T, bool check = std::is_array<T>::value, typename Next = ChainSelectBasic<T>>
		struct ChainSelectArray: ChainSelectForwarder<Next> {};
		
		template<typename T,
			bool Check = std::is_array<T>::value,
			int ArrayExtent = std::extent<T>::value,
			typename Next = ChainSelectArray<T>
		>
		struct ChainSelectStream: ChainSelectForwarder<Next> {};
		
		template<typename T>
		using ChainSelectFirst = ChainSelectStream<T>;
		
		//Select Stream
		template<typename T, typename Next> struct ChainSelectStream<T, true, 0, Next> {
			using InnerNativeType = typename std::remove_extent<T>::type;
			using InnerManagedType = typename ChainSelectFirst<InnerNativeType>::Type;
			using Type = Stream<T, InnerManagedType>;
		};
		
		//Stream
		template<typename TInner, typename TNative>
		struct Stream {
		};
		
		
		//Select Array
		template<typename T, typename Next> struct ChainSelectArray<T, true, Next> {
			using InnerNativeType = typename std::remove_extent<T>::type;
			using InnerManagedType = typename ChainSelectFirst<InnerNativeType>::Type;
			using Type = Array<T, InnerManagedType>;
		};
		
		template<typename T, size_t bMoreDimensions = std::rank<T>::value>
		struct ArrayUnitCounter {
			using InnerType = typename std::remove_extent<T>::type;
			static const size_t nTotalUnits = std::extent<T>::value * ArrayUnitCounter<InnerType>::nTotalUnits;
		};
		
		template<typename T> struct ArrayUnitCounter<T, 1> {
			static const size_t nTotalUnits = std::extent<T>::value;
		};
		//Array
		template<typename TInner, typename TManaged>
		struct Array {
			static const size_t nTotalUnits = ArrayUnitCounter<TInner>::nTotalUnits;
		};
		
		//Select Basic
		template<typename Next> struct ChainSelectBasic<int, Next> {
			using Type = Integer32<int>;
		};
		
		template<typename Next> struct ChainSelectBasic<char, Next> {
			using Type = Character<char>;
		};
		
		
		
		
		//Converts a Native Type to Managed Type
		template<typename Req>
		struct FindManaged {
			using Res = typename ChainSelectFirst<Req>::Type;
		};
		
		template<typename TIn>
		using Managed = typename FindManaged<TIn>::Res;
		
		
	}
	namespace typ {
		
		enum class TypeType {
			Boolean, Integer32, Size, Char, Array, Stream,
		};
		
		using Boolean = bool *;
		using Integer32 = int32_t *;
		using Size = size_t *;
		using Char = char *;
		
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
		
		struct TypeInfo {
			const TypeType typeType;
			TypeInfo(TypeType typeType):typeType(typeType) {}
		};
		
		struct TypeInfoStream: public TypeInfo {
			const TypeInfo & inner;
			TypeInfoStream(const TypeInfo & inner):TypeInfo(TypeType::Stream), inner(inner){}
		};
		
		template<typename Type>
		struct T {
		};
		
		template<typename Inner>
		struct T<Stream<Inner>> {
			static const TypeInfoStream typeInfo;
		};
		
		template<>
		struct T<Boolean> {
			static const TypeInfo typeInfo;
		};
		
		template<>
		struct T<Char> {
			static const TypeInfo typeInfo;
		};
		template<>
		struct T<Integer32> {
			static const TypeInfo typeInfo;
		};
		template<>
		struct T<Size> {
			static const TypeInfo typeInfo;
		};
		
		template<typename Inner>
		const TypeInfoStream T<Stream<Inner>>::typeInfo = {T<Inner>::typeInfo};
		
	}
}

#endif