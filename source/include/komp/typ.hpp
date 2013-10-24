#if !defined(KOMP_TYP_HPP_)
#define KOMP_TYP_HPP_

#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <map>

namespace komp {
	namespace typ {
		
		
		enum class TypeType {
			Boolean, Integer32, Character, Array, Stream,
		};
		
		struct TypeInfo {
			TypeType type;
			const TypeInfo * innerUnitInfo;
		};
		
		
		template<typename TNative>
		struct Void;
		
		template<typename TNative>
		struct Boolean;
		
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
		template<typename TNative, typename TInner>
		struct Stream {
		private:
			static const TypeInfo info;
		public:
			using NativeType = TNative;
			using UnitType = TInner;
			static const TypeInfo & getTypeInfo() { return info; }
		};
		
		template<typename TNative, typename TInner>
		const TypeInfo Stream<TNative, TInner>::info = {
			TypeType::Stream,
			&TInner::getTypeInfo()
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
		template<typename TNative, typename TManaged>
		struct Array {
			static const size_t nTotalUnits = ArrayUnitCounter<TNative>::nTotalUnits;
		};
		
		//Select Basic
		template<typename Next> struct ChainSelectBasic<bool, Next> {
			using Type = Boolean<bool>;
		};
		
		template<typename Next> struct ChainSelectBasic<int, Next> {
			using Type = Integer32<int>;
		};
		
		template<typename Next> struct ChainSelectBasic<char, Next> {
			using Type = Character<char>;
		};
		
		template<typename TNative, TypeType type>
		struct Basic {
		private:
			static const TypeInfo info;
		public:
			using NativeType = TNative;
			static const TypeInfo & getTypeInfo() {return info;}
		};
		
		template<typename TNative, TypeType type>
		const TypeInfo Basic<TNative, type>::info = {
			type,
			nullptr
		};
		
		template<typename TNative>
		struct Boolean: Basic<TNative, TypeType::Boolean> {};
		
		
		template<typename TNative>
		struct Character: Basic<TNative, TypeType::Character> {};
		
		
		template<typename TNative>
		struct Integer32: Basic<TNative, TypeType::Integer32> {};
		
		
		//Converts a Native Type to Managed Type
		template<typename Req>
		struct FindManaged {
			using Res = typename ChainSelectFirst<Req>::Type;
		};
		
		template<typename TIn>
		using Managed = typename FindManaged<TIn>::Res;
		
		
		struct TypeRegistry {
			using TypeID = decltype(std::declval<std::type_info>().hash_code());
			
			template<typename T>
			const TypeInfo & findTypeInfo() {

				auto id = typeid(T).hash_code();
				if (m_typeInfoMap.find(id) == m_typeInfoMap.end()) {
					m_typeInfoMap.emplace(id, Managed<T>::getTypeInfo());
				}
				return m_typeInfoMap[id];
			}
			
		private:
			std::map<TypeID, const TypeInfo &> m_typeInfoMap;
		};
	}
}

#endif