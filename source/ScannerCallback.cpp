#include "ScannerCallback.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include "stdio.h"
using namespace komp;

ScannerCallback::ScannerCallback(Context & ctx):m_ctx(ctx){
	
}

ScannerCallback::~ScannerCallback() {
	
}

int ScannerCallback::onToken(TokenType tokenType, YYSTYPE * stype, YYLTYPE * ltype, const char * tokenStart, int tokenLength, int lineNumber, int columnNumber) {
	
	ltype->first_line = ltype->last_line = lineNumber;
	ltype->first_column = columnNumber;
	ltype->last_column = columnNumber + tokenLength;
	
	int iRet;
	
	switch(tokenType) {
		case TokenType::error:
			iRet = 0;
			break;
		case TokenType::endOfStream:
			iRet = 0;
			break;
		case TokenType::integer:
			stype->node.integerValue = atoi(tokenStart);
			iRet = TERM_INTEGER;
			break;
		case TokenType::operatorPlus:
			iRet = TERM_OP_PLUS;
			break;
		case TokenType::statementEnd:
			iRet = TERM_STATEMENT;
			break;
	}
	
	return iRet;
}
