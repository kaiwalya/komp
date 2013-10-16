%{
	#include "ScannerCallback.hpp"
	#include "parser.hpp"
	using TT = komp::ScannerCallback::TokenType;
%}
%option reentrant
%option noyywrap
%option extra-type="komp::ScannerCallback *"
%option bison-bridge
%option bison-locations

WHITESPACE	[ \t\n\r]
DIGIT		[0-9]
DIGITS		{DIGIT}+
OP_PLUS		[\+]
TERMINAL	;

%%

{DIGITS}				{return yyextra->onToken(TT::integer, yylval, yylloc, yytext, yyleng, yylineno, yycolumn);}
[\+]					{return yyextra->onToken(TT::operatorPlus, yylval, yylloc, yytext, yyleng, yylineno, yycolumn);}
{TERMINAL}				{return yyextra->onToken(TT::statementEnd, yylval, yylloc, yytext, yyleng, yylineno, yycolumn);}

	/* Ignore WHITESPACE */
{WHITESPACE}			{}


<<EOF>>		 			{return yyextra->onToken(TT::endOfStream, yylval, yylloc, yytext, yyleng, yylineno, yycolumn); yyterminate();}
[\0]					{return yyextra->onToken(TT::endOfStream, yylval, yylloc, yytext, yyleng, yylineno, yycolumn); yyterminate();}

	/* Everything else is an error */
.						{return yyextra->onToken(TT::error, yylval, yylloc, yytext, yyleng, yylineno, yycolumn);}
%%

