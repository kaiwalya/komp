#if !defined(SCANNER_CALLBACK_HPP)
#define SCANNER_CALLBACK_HPP

#include "Grammar.hpp"
#include "parser.hpp"


namespace komp{

	class ScannerCallback{
	public:
		using TokenType = komp::grammar::TokenType;
		
		using Context = void *;
		virtual int onToken(TokenType tokenType, YYSTYPE * stype, YYLTYPE * ltype, const char * tokenStart, int tokenLength, int lineNumber, int columnNumber);
		ScannerCallback(Context & ctx);
		virtual ~ScannerCallback();
	private:
		Context & m_ctx;
	};
}

#endif