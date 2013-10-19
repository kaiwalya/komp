#include <iostream>

#include "ParserCallback.hpp"


int komp::komp_ParserCallback_lex(YYSTYPE * stype, YYLTYPE * ltype, ParserCallback * parserCallback) {

	return parserCallback->lex(stype, ltype);
}


void komp::komp_ParserCallback_error(YYLTYPE * location, komp::ParserCallback *parserCallback, const char *errorString ) {
	std::cerr << "Error:"
		<< errorString
		<< ". (ln:"
		<< location->first_line
		<< ":col:"
		<< location->first_column
		<< ") - (ln:"
		<< location->last_line
		<< ":col:"
		<< location->last_column
		<< ")"
		<< std::endl;
}


using namespace komp;

ParserCallback::ParserCallback(): scannerCallback(scanner){
	yylex_init_extra(&scannerCallback, &scanner);
	static const char toScan[] = {"31 + 71; 31; 77; 31;\n\n"};
	//yy_scan_string(toScan, scanner);
	komp_ParserCallback_parse(this);
	yylex_destroy(scanner);
}

ParserCallback::~ParserCallback() {
	
}

int ParserCallback::lex(YYSTYPE * stype, YYLTYPE * ltype) {
	return yylex(stype, ltype, scanner);
}