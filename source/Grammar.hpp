#if !defined(GRAMMAR_HPP_)
#define GRAMMAR_HPP_

namespace komp {
	namespace grammar {
		
		enum class TokenType {
			error,
			endOfStream,
			integer,
			operatorPlus,
			statementEnd,
		};
		
		struct Node {
			TokenType token;
			union {
				int integerValue;
			};
		};
	}
}

#endif