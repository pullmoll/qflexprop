/*****************************************************************************
 *
 * Qt5 Propeller 2 string identifiers and tokens
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "propconst.h"

/**
 * @file Propeller 2 token class
 *
 * This lists (a subset of) token strings used with Propeller 2
 * editor's supported languages.
 *
 * TODO:
 *
 * Classify tokens per language so that syntax highlighting
 * can adapt to the language in the current editor.
 *
 * Replace descriptions with meaningful text that could be
 * displayed as QToolTip whenever the mouse cursor is placed over a
 * keyword for some time.
 */

CPropToken::CPropToken(const PropToken t, const QString& name, const QString& desc)
    : m_token(t)
    , m_name(name)
    , m_desc(desc)
{

}

PropToken CPropToken::tok() const
{
    return m_token;
}

const QString& CPropToken::name() const
{
    return m_name;
}

const QString& CPropToken::desc() const
{
    return m_desc;
}

CPropTokens::CPropTokens()
    : m_tokens()
{
    add(TOK_CPP_IF, "#if", "Preprocessor condition");
    add(TOK_CPP_ELSE, "#else", "Preprocessor condition");
    add(TOK_CPP_ENDIF, "#endif", "Preprocessor condition");
    add(TOK_CPP_IFDEF, "#ifdef", "Preprocessor condition");
    add(TOK_CPP_IFNDEF, "#ifndef", "Preprocessor condition");
    add(TOK_CPP_ELIF, "#elif", "Preprocessor condition");
    add(TOK_CPP_ELIFDEF, "#elifdef", "Preprocessor condition");
    add(TOK_CPP_ELIFNDEF, "#elifndef", "Preprocessor condition");
    add(TOK_CPP_DEFINE, "#define", "Preprocessor definition");
    add(TOK_CPP_UNDEF, "#undef", "Preprocessor definition remove");

    add(TOK_CON, "CON", "CON");
    add(TOK_VAR, "VAR", "VAR");
    add(TOK_DAT, "DAT", "DAT");
    add(TOK_PUB, "PUB", "PUB");
    add(TOK_PRI, "PRI", "PRI");
    add(TOK_OBJ, "OBJ", "OBJ");
    add(TOK_ASM, "ASM", "ASM");
    add(TOK_ENDASM, "ENDASM", "ENDASM");
    add(TOK_END, "END", "END");
    add(TOK_INLINECCODE, "CCODE", "CCODE");

    add(TOK_BYTE, "BYTE", "BYTE");
    add(TOK_WORD, "WORD", "WORD");
    add(TOK_LONG, "LONG", "LONG");
    add(TOK_FVAR, "FVAR", "FVAR");
    add(TOK_FVARS, "FVARS", "FVARS");
    add(TOK_ASMCLK, "ASMCLK", "ASMCLK");

    add(TOK_INSTR, nullptr, "instruction");
    add(TOK_INSTRMODIFIER, nullptr, "instruction modifier");
    add(TOK_HWREG, nullptr, "hardware register");
    add(TOK_ORG, "ORG", "ORG");
    add(TOK_ORGH, "ORGH", "ORGH");
    add(TOK_ORGF, "ORGF", "ORGF");
    add(TOK_RES, "RES", "RES");
    add(TOK_FIT, "FIT", "FIT");
    add(TOK_ALIGNL, "ALIGNL", "ALIGNL");
    add(TOK_ALIGNW, "ALIGNW", "ALIGNW");

    add(TOK_REPEAT, "REPEAT", "REPEAT");
    add(TOK_FROM, "FROM", "FROM");
    add(TOK_TO, "TO", "TO");
    add(TOK_STEP, "STEP", "STEP");
    add(TOK_WHILE, "WHILE", "WHILE");
    add(TOK_UNTIL, "UNTIL", "UNTIL");
    add(TOK_IF, "IF", "IF");
    add(TOK_IFNOT, "IFNOT", "IFNOT");
    add(TOK_ELSE, "ELSE", "ELSE");
    add(TOK_ELSEIF, "ELSEIF", "ELSEIF");
    add(TOK_ELSEIFNOT, "ELSEIFNOT", "ELSEIFNOT");
    add(TOK_THEN, "THEN", "THEN");
    add(TOK_ENDIF, "ENDIF", "ENDIF");

    add(TOK_LOOKDOWN, "LOOKDOWN", "LOOKDOWN");
    add(TOK_LOOKDOWNZ, "LOOKDOWNZ", "LOOKDOWNZ");
    add(TOK_LOOKUP, "LOOKUP", "LOOKUP");
    add(TOK_LOOKUPZ, "LOOKUPZ", "LOOKUPZ");
    add(TOK_COGINIT2, "COGINIT", "COGINIT");
    add(TOK_COGNEW, "COGNEW", "COGNEW");

    add(TOK_CASE, "CASE", "CASE");
    add(TOK_CASE_FAST, "CASE_FAST", "CASE_FAST");
    add(TOK_OTHER, "OTHER", "OTHER");

    add(TOK_QUIT, "QUIT", "QUIT");
    add(TOK_NEXT, "NEXT", "NEXT");

    add(TOK_ALLOCA, "__BUILTIN_ALLOCA", "__BUILTIN_ALLOCA");

    add(TOK_ABORT, "ABORT", "ABORT");
    add(TOK_RESULT, "RESULT", "RESULT");
    add(TOK_RETURN, "RETURN", "RETURN");
    add(TOK_INDENT, nullptr, "indentation");
    add(TOK_OUTDENT, nullptr, "lack of indentation");
    add(TOK_EOLN, nullptr, "end of line");
    add(TOK_EOF, nullptr, "end of file");
    add(TOK_DOTS, "..", "..");
    add(TOK_HERE, "$", "$");
    add(TOK_STRINGPTR, nullptr, "STRING");
    add(TOK_FILE, nullptr, "FILE");

    add(TOK_NOP, "NOP", "No operation");
    add(TOK_ROR, "ROR", "ROR");
    add(TOK_ROL, "ROL", "ROL");
    add(TOK_SHR, "SHR", "SHR");
    add(TOK_SHL, "SHL", "SHL");
    add(TOK_RCR, "RCR", "RCR");
    add(TOK_RCL, "RCL", "RCL");
    add(TOK_SAR, "SAR", "SAR");
    add(TOK_SAL, "SAL", "SAL");
    add(TOK_ADD, "ADD", "ADD");
    add(TOK_ADDX, "ADDX", "ADDX");
    add(TOK_ADDS, "ADDS", "ADDS");
    add(TOK_ADDSX, "ADDSX", "ADDSX");
    add(TOK_SUB, "SUB", "SUB");
    add(TOK_SUBX, "SUBX", "SUBX");
    add(TOK_SUBS, "SUBS", "SUBS");
    add(TOK_SUBSX, "SUBSX", "SUBSX");
    add(TOK_CMP, "CMP", "CMP");
    add(TOK_CMPX, "CMPX", "CMPX");
    add(TOK_CMPS, "CMPS", "CMPS");
    add(TOK_CMPSX, "CMPSX", "CMPSX");
    add(TOK_CMPR, "CMPR", "CMPR");
    add(TOK_CMPM, "CMPM", "CMPM");
    add(TOK_SUBR, "SUBR", "SUBR");
    add(TOK_CMPSUB, "CMPSUB", "CMPSUB");
    add(TOK_FGE, "FGE", "FGE");
    add(TOK_FLE, "FLE", "FLE");
    add(TOK_FGES, "FGES", "FGES");
    add(TOK_FLES, "FLES", "FLES");
    add(TOK_SUMC, "SUMC", "SUMC");
    add(TOK_SUMNC, "SUMNC", "SUMNC");
    add(TOK_SUMZ, "SUMZ", "SUMZ");
    add(TOK_SUMNZ, "SUMNZ", "SUMNZ");
    add(TOK_TESTB, "TESTB", "TESTB");
    add(TOK_TESTBN, "TESTBN", "TESTBN");
    add(TOK_BITL, "BITL", "BITL");
    add(TOK_BITH, "BITH", "BITH");
    add(TOK_BITC, "BITC", "BITC");
    add(TOK_BITNC, "BITNC", "BITNC");
    add(TOK_BITZ, "BITZ", "BITZ");
    add(TOK_BITNZ, "BITNZ", "BITNZ");
    add(TOK_BITRND, "BITRND", "BITRND");
    add(TOK_BITNOT, "BITNOT", "BITNOT");
    add(TOK_AND, "AND", "AND");
    add(TOK_ANDN, "ANDN", "ANDN");
    add(TOK_OR, "OR", "OR");
    add(TOK_XOR, "XOR", "XOR");
    add(TOK_MUXC, "MUXC", "MUXC");
    add(TOK_MUXNC, "MUXNC", "MUXNC");
    add(TOK_MUXZ, "MUXZ", "MUXZ");
    add(TOK_MUXNZ, "MUXNZ", "MUXNZ");
    add(TOK_MOV, "MOV", "MOV");
    add(TOK_NOT, "NOT", "NOT");
    add(TOK_ABS, "ABS", "ABS");
    add(TOK_NEG, "NEG", "NEG");
    add(TOK_NEGC, "NEGC", "NEGC");
    add(TOK_NEGNC, "NEGNC", "NEGNC");
    add(TOK_NEGZ, "NEGZ", "NEGZ");
    add(TOK_NEGNZ, "NEGNZ", "NEGNZ");
    add(TOK_INCMOD, "INCMOD", "INCMOD");
    add(TOK_DECMOD, "DECMOD", "DECMOD");
    add(TOK_ZEROX, "ZEROX", "ZEROX");
    add(TOK_SIGNX, "SIGNX", "SIGNX");
    add(TOK_ENCOD, "ENCOD", "ENCOD");
    add(TOK_ONES, "ONES", "ONES");
    add(TOK_TEST, "TEST", "TEST");
    add(TOK_TESTN, "TESTN", "TESTN");
    add(TOK_SETNIB, "SETNIB", "SETNIB");
    add(TOK_GETNIB, "GETNIB", "GETNIB");
    add(TOK_ROLNIB, "ROLNIB", "ROLNIB");
    add(TOK_SETBYTE, "SETBYTE", "SETBYTE");
    add(TOK_GETBYTE, "GETBYTE", "GETBYTE");
    add(TOK_ROLBYTE, "ROLBYTE", "ROLBYTE");
    add(TOK_SETWORD, "SETWORD", "SETWORD");
    add(TOK_GETWORD, "GETWORD", "GETWORD");
    add(TOK_ROLWORD, "ROLWORD", "ROLWORD");
    add(TOK_ALTSN, "ALTSN", "ALTSN");
    add(TOK_ALTGN, "ALTGN", "ALTGN");
    add(TOK_ALTSB, "ALTSB", "ALTSB");
    add(TOK_ALTGB, "ALTGB", "ALTGB");
    add(TOK_ALTSW, "ALTSW", "ALTSW");
    add(TOK_ALTGW, "ALTGW", "ALTGW");
    add(TOK_ALTR, "ALTR", "ALTR");
    add(TOK_ALTD, "ALTD", "ALTD");
    add(TOK_ALTS, "ALTS", "ALTS");
    add(TOK_ALTB, "ALTB", "ALTB");
    add(TOK_ALTI, "ALTI", "ALTI");
    add(TOK_SETR, "SETR", "SETR");
    add(TOK_SETD, "SETD", "SETD");
    add(TOK_SETS, "SETS", "SETS");
    add(TOK_DECOD, "DECOD", "DECOD");
    add(TOK_BMASK, "BMASK", "BMASK");
    add(TOK_CRCBIT, "CRCBIT", "CRCBIT");
    add(TOK_CRCNIB, "CRCNIB", "CRCNIB");
    add(TOK_MUXNITS, "MUXNITS", "MUXNITS");
    add(TOK_MUXNIBS, "MUXNIBS", "MUXNIBS");
    add(TOK_MUXQ, "MUXQ", "MUXQ");
    add(TOK_MOVBYTS, "MOVBYTS", "MOVBYTS");
    add(TOK_MUL, "MUL", "MUL");
    add(TOK_MULS, "MULS", "MULS");
    add(TOK_SCA, "SCA", "SCA");
    add(TOK_SCAS, "SCAS", "SCAS");
    add(TOK_ADDPIX, "ADDPIX", "ADDPIX");
    add(TOK_MULPIX, "MULPIX", "MULPIX");
    add(TOK_BLNPIX, "BLNPIX", "BLNPIX");
    add(TOK_MIXPIX, "MIXPIX", "MIXPIX");
    add(TOK_ADDCT1, "ADDCT1", "ADDCT1");
    add(TOK_ADDCT2, "ADDCT2", "ADDCT2");
    add(TOK_ADDCT3, "ADDCT3", "ADDCT3");
    add(TOK_WMLONG, "WMLONG", "WMLONG");
    add(TOK_RQPIN, "RQPIN", "RQPIN");
    add(TOK_RDPIN, "RDPIN", "RDPIN");
    add(TOK_RDLUT, "RDLUT", "RDLUT");
    add(TOK_RDBYTE, "RDBYTE", "RDBYTE");
    add(TOK_RDWORD, "RDWORD", "RDWORD");
    add(TOK_RDLONG, "RDLONG", "RDLONG");
    add(TOK_POPA, "POPA", "POPA");
    add(TOK_POPB, "POPB", "POPB");
    add(TOK_CALLD, "CALLD", "CALLD");
    add(TOK_RESI3, "RESI3", "RESI3");
    add(TOK_RESI2, "RESI2", "RESI2");
    add(TOK_RESI1, "RESI1", "RESI1");
    add(TOK_RESI0, "RESI0", "RESI0");
    add(TOK_RETI3, "RETI3", "RETI3");
    add(TOK_RETI2, "RETI2", "RETI2");
    add(TOK_RETI1, "RETI1", "RETI1");
    add(TOK_RETI0, "RETI0", "RETI0");
    add(TOK_CALLPA, "CALLPA", "CALLPA");
    add(TOK_CALLPB, "CALLPB", "CALLPB");
    add(TOK_DJZ, "DJZ", "DJZ");
    add(TOK_DJNZ, "DJNZ", "DJNZ");
    add(TOK_DJF, "DJF", "DJF");
    add(TOK_DJNF, "DJNF", "DJNF");
    add(TOK_IJZ, "IJZ", "IJZ");
    add(TOK_IJNZ, "IJNZ", "IJNZ");
    add(TOK_TJZ, "TJZ", "TJZ");
    add(TOK_TJNZ, "TJNZ", "TJNZ");
    add(TOK_TJF, "TJF", "TJF");
    add(TOK_TJNF, "TJNF", "TJNF");
    add(TOK_TJS, "TJS", "TJS");
    add(TOK_TJNS, "TJNS", "TJNS");
    add(TOK_TJV, "TJV", "TJV");
    add(TOK_JINT, "JINT", "JINT");
    add(TOK_JCT1, "JCT1", "JCT1");
    add(TOK_JCT2, "JCT2", "JCT2");
    add(TOK_JCT3, "JCT3", "JCT3");
    add(TOK_JSE1, "JSE1", "JSE1");
    add(TOK_JSE2, "JSE2", "JSE2");
    add(TOK_JSE3, "JSE3", "JSE3");
    add(TOK_JSE4, "JSE4", "JSE4");
    add(TOK_JPAT, "JPAT", "JPAT");
    add(TOK_JFBW, "JFBW", "JFBW");
    add(TOK_JXMT, "JXMT", "JXMT");
    add(TOK_JXFI, "JXFI", "JXFI");
    add(TOK_JXRO, "JXRO", "JXRO");
    add(TOK_JXRL, "JXRL", "JXRL");
    add(TOK_JATN, "JATN", "JATN");
    add(TOK_JQMT, "JQMT", "JQMT");
    add(TOK_JNINT, "JNINT", "JNINT");
    add(TOK_JNCT1, "JNCT1", "JNCT1");
    add(TOK_JNCT2, "JNCT2", "JNCT2");
    add(TOK_JNCT3, "JNCT3", "JNCT3");
    add(TOK_JNSE1, "JNSE1", "JNSE1");
    add(TOK_JNSE2, "JNSE2", "JNSE2");
    add(TOK_JNSE3, "JNSE3", "JNSE3");
    add(TOK_JNSE4, "JNSE4", "JNSE4");
    add(TOK_JNPAT, "JNPAT", "JNPAT");
    add(TOK_JNFBW, "JNFBW", "JNFBW");
    add(TOK_JNXMT, "JNXMT", "JNXMT");
    add(TOK_JNXFI, "JNXFI", "JNXFI");
    add(TOK_JNXRO, "JNXRO", "JNXRO");
    add(TOK_JNXRL, "JNXRL", "JNXRL");
    add(TOK_JNATN, "JNATN", "JNATN");
    add(TOK_JNQMT, "JNQMT", "JNQMT");
    add(TOK_SETPAT, "SETPAT", "SETPAT");
    add(TOK_AKPIN, "AKPIN", "AKPIN");
    add(TOK_WRPIN, "WRPIN", "WRPIN");
    add(TOK_WXPIN, "WXPIN", "WXPIN");
    add(TOK_WYPIN, "WYPIN", "WYPIN");
    add(TOK_WRLUT, "WRLUT", "WRLUT");
    add(TOK_WRBYTE, "WRBYTE", "WRBYTE");
    add(TOK_WRWORD, "WRWORD", "WRWORD");
    add(TOK_WRLONG, "WRLONG", "WRLONG");
    add(TOK_PUSHA, "PUSHA", "PUSHA");
    add(TOK_PUSHB, "PUSHB", "PUSHB");
    add(TOK_RDFAST, "RDFAST", "RDFAST");
    add(TOK_WRFAST, "WRFAST", "WRFAST");
    add(TOK_FBLOCK, "FBLOCK", "FBLOCK");
    add(TOK_XINIT, "XINIT", "XINIT");
    add(TOK_XSTOP, "XSTOP", "XSTOP");
    add(TOK_XZERO, "XZERO", "XZERO");
    add(TOK_XCONT, "XCONT", "XCONT");
    add(TOK_REP, "REP", "REP");
    add(TOK_COGINIT, "COGINIT", "COGINIT");
    add(TOK_QMUL, "QMUL", "QMUL");
    add(TOK_QDIV, "QDIV", "QDIV");
    add(TOK_QFRAC, "QFRAC", "QFRAC");
    add(TOK_QSQRT, "QSQRT", "QSQRT");
    add(TOK_QROTATE, "QROTATE", "QROTATE");
    add(TOK_QVECTOR, "QVECTOR", "QVECTOR");
    add(TOK_HUBSET, "HUBSET", "HUBSET");
    add(TOK_COGID, "COGID", "COGID");
    add(TOK_COGSTOP, "COGSTOP", "COGSTOP");
    add(TOK_LOCKNEW, "LOCKNEW", "LOCKNEW");
    add(TOK_LOCKRET, "LOCKRET", "LOCKRET");
    add(TOK_LOCKTRY, "LOCKTRY", "LOCKTRY");
    add(TOK_LOCKREL, "LOCKREL", "LOCKREL");
    add(TOK_QLOG, "QLOG", "QLOG");
    add(TOK_QEXP, "QEXP", "QEXP");
    add(TOK_RFBYTE, "RFBYTE", "RFBYTE");
    add(TOK_RFWORD, "RFWORD", "RFWORD");
    add(TOK_RFLONG, "RFLONG", "RFLONG");
    add(TOK_RFVAR, "RFVAR", "RFVAR");
    add(TOK_RFVARS, "RFVARS", "RFVARS");
    add(TOK_WFBYTE, "WFBYTE", "WFBYTE");
    add(TOK_WFWORD, "WFWORD", "WFWORD");
    add(TOK_WFLONG, "WFLONG", "WFLONG");
    add(TOK_GETQX, "GETQX", "GETQX");
    add(TOK_GETQY, "GETQY", "GETQY");
    add(TOK_GETCT, "GETCT", "GETCT");
    add(TOK_GETRND, "GETRND", "GETRND");
    add(TOK_SETDACS, "SETDACS", "SETDACS");
    add(TOK_SETXFRQ, "SETXFRQ", "SETXFRQ");
    add(TOK_FETXACC, "FETXACC", "FETXACC");
    add(TOK_WAITX, "WAITX", "WAITX");
    add(TOK_SETSE1, "SETSE1", "SETSE1");
    add(TOK_SETSE2, "SETSE2", "SETSE2");
    add(TOK_SETSE3, "SETSE3", "SETSE3");
    add(TOK_SETSE4, "SETSE4", "SETSE4");
    add(TOK_POLLINT, "POLLINT", "POLLINT");
    add(TOK_POLLCT1, "POLLCT1", "POLLCT1");
    add(TOK_POLLCT2, "POLLCT2", "POLLCT2");
    add(TOK_POLLCT3, "POLLCT3", "POLLCT3");
    add(TOK_POLLSE1, "POLLSE1", "POLLSE1");
    add(TOK_POLLSE2, "POLLSE2", "POLLSE2");
    add(TOK_POLLSE3, "POLLSE3", "POLLSE3");
    add(TOK_POLLSE4, "POLLSE4", "POLLSE4");
    add(TOK_POLLPAT, "POLLPAT", "POLLPAT");
    add(TOK_POLLFBW, "POLLFBW", "POLLFBW");
    add(TOK_POLLXMT, "POLLXMT", "POLLXMT");
    add(TOK_POLLXFI, "POLLXFI", "POLLXFI");
    add(TOK_POLLXRO, "POLLXRO", "POLLXRO");
    add(TOK_POLLXRL, "POLLXRL", "POLLXRL");
    add(TOK_POLLATN, "POLLATN", "POLLATN");
    add(TOK_POLLQMT, "POLLQMT", "POLLQMT");
    add(TOK_WAITINT, "WAITINT", "WAITINT");
    add(TOK_WAITCT1, "WAITCT1", "WAITCT1");
    add(TOK_WAITCT2, "WAITCT2", "WAITCT2");
    add(TOK_WAITCT3, "WAITCT3", "WAITCT3");
    add(TOK_WAITSE1, "WAITSE1", "WAITSE1");
    add(TOK_WAITSE2, "WAITSE2", "WAITSE2");
    add(TOK_WAITSE3, "WAITSE3", "WAITSE3");
    add(TOK_WAITSE4, "WAITSE4", "WAITSE4");
    add(TOK_WAITPAT, "WAITPAT", "WAITPAT");
    add(TOK_WAITFBW, "WAITFBW", "WAITFBW");
    add(TOK_WAITXMT, "WAITXMT", "WAITXMT");
    add(TOK_WAITXFI, "WAITXFI", "WAITXFI");
    add(TOK_WAITXRO, "WAITXRO", "WAITXRO");
    add(TOK_WAITXRL, "WAITXRL", "WAITXRL");
    add(TOK_WAITATN, "WAITATN", "WAITATN");
    add(TOK_ALLOWI, "ALLOWI", "ALLOWI");
    add(TOK_STALLI, "STALLI", "STALLI");
    add(TOK_TRGINT1, "TRGINT1", "TRGINT1");
    add(TOK_TRGINT2, "TRGINT2", "TRGINT2");
    add(TOK_TRGINT3, "TRGINT3", "TRGINT3");
    add(TOK_NIXINT1, "NIXINT1", "NIXINT1");
    add(TOK_NIXINT2, "NIXINT2", "NIXINT2");
    add(TOK_NIXINT3, "NIXINT3", "NIXINT3");
    add(TOK_SETINT1, "SETINT1", "SETINT1");
    add(TOK_SETINT2, "SETINT2", "SETINT2");
    add(TOK_SETINT3, "SETINT3", "SETINT3");
    add(TOK_SETQ, "SETQ", "SETQ");
    add(TOK_SETQ2, "SETQ2", "SETQ2");
    add(TOK_PUSH, "PUSH", "PUSH");
    add(TOK_POP, "POP", "POP");
    add(TOK_JMP, "JMP", "JMP");
    add(TOK_CALL, "CALL", "CALL");
    add(TOK_RET, "RET", "RET");
    add(TOK_CALLA, "CALLA", "CALLA");
    add(TOK_RETA, "RETA", "RETA");
    add(TOK_CALLB, "CALLB", "CALLB");
    add(TOK_RETB, "RETB", "RETB");
    add(TOK_JMPREL, "JMPREL", "JMPREL");
    add(TOK_SKIP, "SKIP", "SKIP");
    add(TOK_SKIPF, "SKIPF", "SKIPF");
    add(TOK_EXECF, "EXECF", "EXECF");
    add(TOK_GETPTR, "GETPTR", "GETPTR");
    add(TOK_GETBRK, "GETBRK", "GETBRK");
    add(TOK_COGBRK, "COGBRK", "COGBRK");
    add(TOK_BRK, "BRK", "BRK");
    add(TOK_SETLUTS, "SETLUTS", "SETLUTS");
    add(TOK_SETCY, "SETCY", "SETCY");
    add(TOK_SETCI, "SETCI", "SETCI");
    add(TOK_SETCQ, "SETCQ", "SETCQ");
    add(TOK_SETCFRQ, "SETCFRQ", "SETCFRQ");
    add(TOK_SETCMOD, "SETCMOD", "SETCMOD");
    add(TOK_SETPIV, "SETPIV", "SETPIV");
    add(TOK_SETPIX, "SETPIX", "SETPIX");
    add(TOK_COGATN, "COGATN", "COGATN");
    add(TOK_TESTP, "TESTP", "TESTP");
    add(TOK_TESTPN, "TESTPN", "TESTPN");
    add(TOK_DIRL, "DIRL", "DIRL");
    add(TOK_DIRH, "DIRH", "DIRH");
    add(TOK_DIRC, "DIRC", "DIRC");
    add(TOK_DIRNC, "DIRNC", "DIRNC");
    add(TOK_DIRZ, "DIRZ", "DIRZ");
    add(TOK_DIRNZ, "DIRNZ", "DIRNZ");
    add(TOK_DIRRND, "DIRRND", "DIRRND");
    add(TOK_DIRNOT, "DIRNOT", "DIRNOT");
    add(TOK_OUTL, "OUTL", "OUTL");
    add(TOK_OUTH, "OUTH", "OUTH");
    add(TOK_OUTC, "OUTC", "OUTC");
    add(TOK_OUTNC, "OUTNC", "OUTNC");
    add(TOK_OUTZ, "OUTZ", "OUTZ");
    add(TOK_OUTNZ, "OUTNZ", "OUTNZ");
    add(TOK_OUTRND, "OUTRND", "OUTRND");
    add(TOK_OUTNOT, "OUTNOT", "OUTNOT");
    add(TOK_FLTL, "FLTL", "FLTL");
    add(TOK_FLTH, "FLTH", "FLTH");
    add(TOK_FLTC, "FLTC", "FLTC");
    add(TOK_FLTNC, "FLTNC", "FLTNC");
    add(TOK_FLTZ, "FLTZ", "FLTZ");
    add(TOK_FLTNZ, "FLTNZ", "FLTNZ");
    add(TOK_FLTRND, "FLTRND", "FLTRND");
    add(TOK_FLTNOT, "FLTNOT", "FLTNOT");
    add(TOK_DRVL, "DRVL", "DRVL");
    add(TOK_DRVH, "DRVH", "DRVH");
    add(TOK_DRVC, "DRVC", "DRVC");
    add(TOK_DRVNC, "DRVNC", "DRVNC");
    add(TOK_DRVZ, "DRVZ", "DRVZ");
    add(TOK_DRVNZ, "DRVNZ", "DRVNZ");
    add(TOK_DRVRND, "DRVRND", "DRVRND");
    add(TOK_DRVNOT, "DRVNOT", "DRVNOT");
    add(TOK_SPLITB, "SPLITB", "SPLITB");
    add(TOK_MERGEB, "MERGEB", "MERGEB");
    add(TOK_SPLITW, "SPLITW", "SPLITW");
    add(TOK_MERGEW, "MERGEW", "MERGEW");
    add(TOK_SEUSSR, "SEUSSR", "SEUSSR");
    add(TOK_RGBSQZ, "RGBSQZ", "RGBSQZ");
    add(TOK_RGBEXP, "RGBEXP", "RGBEXP");
    add(TOK_XORO32, "XORO32", "XORO32");
    add(TOK_REV, "REV", "REV");
    add(TOK_RCZR, "RCZR", "RCZR");
    add(TOK_RCZL, "RCZL", "RCZL");
    add(TOK_WRC, "WRC", "WRC");
    add(TOK_RCNC, "RCNC", "RCNC");
    add(TOK_WRZ, "WRZ", "WRZ");
    add(TOK_RCNZ, "RCNZ", "RCNZ");
    add(TOK_MODCZ, "MODCZ", "MODCZ");
    add(TOK_MODC, "MODC", "MODC");
    add(TOK_MODZ, "MODZ", "MODZ");
    add(TOK_SETSCP, "SETSCP", "SETSCP");
    add(TOK_GETSCP, "GETSCP", "GETSCP");
    add(TOK_LOC, "LOC", "LOC");
    add(TOK_AUGS, "AUGS", "AUGS");
    add(TOK_AUGD, "AUGD", "AUGD");
    add(TOK__RET_, "_RET_", "_RET_");

    add(TOK_ASSIGN, ":=", ":=");
    add(TOK_OP_XOR, "^^", "XOR (^^)");
    add(TOK_OP_OR, "||", "OR (||)");
    add(TOK_OP_AND, "&&", "AND (&&)");
    add(TOK_OP_ORELSE, "__ORELSE__", "__ORELSE__");
    add(TOK_OP_ANDTHEN, "__ANDTHEN__", "__ANDTHEN__");
    add(TOK_OP_GE, "=>", "=>");
    add(TOK_OP_LE, "=<", "=<");
    add(TOK_OP_GEU, "+=>", "+=>");
    add(TOK_OP_LEU, "+=<", "+=<");
    add(TOK_OP_GTU, "+>", "+>");
    add(TOK_OP_LTU, "+<", "+<");
    add(TOK_OP_NE, "<>", "<>");
    add(TOK_OP_EQ, "==", "==");
    add(TOK_OP_SGNCOMP, "<=>", "<=>");
    add(TOK_OP_LIMITMIN, "#>", "#>");
    add(TOK_OP_LIMITMAX, "<#", "<#");
    add(TOK_OP_REMAINDER, "//", "//");
    add(TOK_OP_UNSDIV, "+/", "+/");
    add(TOK_OP_UNSMOD, "+//", "+//");
    add(TOK_OP_FRAC, "FRAC", "FRAC");
    add(TOK_OP_HIGHMULT, "**", "**");
    add(TOK_OP_SCAS, "SCAS", "SCAS");
    add(TOK_OP_UNSHIGHMULT, "+**", "SCA (+**)");
    add(TOK_OP_ROTR, "->", "ROR (->)");
    add(TOK_OP_ROTL, "<-", "ROL (<-)");
    add(TOK_OP_SHL, "<<", "<<");
    add(TOK_OP_SHR, ">>", ">>");
    add(TOK_OP_SAR, "~>", "SAR (~>)");
    add(TOK_OP_REV, "><", "><");
    add(TOK_OP_REV2, "REV", "REV");
    add(TOK_OP_ADDBITS, "ADDBITS", "ADDBITS");
    add(TOK_OP_ADDPINS, "ADDPINS", "ADDPINS");
    add(TOK_OP_NEGATE, "-", "-");
    add(TOK_OP_BIT_NOT, "!", "!");
    add(TOK_OP_SQRT, "^^", "SQRT (^^)");
    add(TOK_OP_ABS, "||", "ABS (||)");
    add(TOK_OP_DECODE, "|<", "DECOD (|<)");
    add(TOK_OP_ENCODE, ">|", ">|");
    add(TOK_OP_ENCODE2, "ENCOD", "ENCOD");
    add(TOK_OP_NOT, "!!", "NOT (!!)");
    add(TOK_OP_DOUBLETILDE, "~~", "~~");
    add(TOK_OP_INCREMENT, "++", "++");
    add(TOK_OP_DECREMENT, "--", "--");
    add(TOK_OP_DOUBLEAT, "@@", "@@");
    add(TOK_OP_TRIPLEAT, "@@@", "@@@");
    add(TOK_OP_FLOAT, nullptr, "floating point number");
    add(TOK_OP_TRUNC, "TRUNC", "TRUNC");
    add(TOK_OP_ROUND, "ROUND", "ROUND");
    add(TOK_CONSTANT, nullptr, "constant");
    add(TOK_RANDOM, "??", "??");
    add(TOK_EMPTY, nullptr, "empty assignment marker _");
    add(TOK_OP_SIGNX, "SIGNX", "SIGNX");
    add(TOK_OP_ZEROX, "ZEROX", "ZEROX");
    add(TOK_OP_ONES, "ONES", "ONES");
    add(TOK_OP_BMASK, "BMASK", "BMASK");
    add(TOK_OP_QLOG, "QLOG", "QLOG");
    add(TOK_OP_QEXP, "QEXP", "QEXP");
    add(TOK_OP_DEBUG, "DEBUG", "DEBUG");
    add(TOK_LOOK_SEP, ":", ":");
}

void CPropTokens::add(const PropToken t, const QString& name, const QString& desc)
{
    m_tokens.insert(t, CPropToken(t, name, desc));
}

QStringList CPropTokens::list(const QList<PropToken>& filter) const
{
    QStringList list;
    if (filter.isEmpty()) {
	list.reserve(TOK_);
	foreach(const CPropToken& tok, m_tokens.values())
	    if (!tok.name().isNull())
		list += tok.name();
    } else {
	list.reserve(filter.count());
	foreach(const PropToken t, filter) {
	    const CPropToken& tok = m_tokens.value(t);
	    if (!tok.name().isNull())
		list += tok.name();
	}
    }
    return list;
}

QStringList CPropTokens::list_esc(const QList<PropToken>& filter) const
{
    QStringList list;
    if (filter.isEmpty()) {
	list.reserve(TOK_);
	foreach(const CPropToken& tok, m_tokens.values())
	    if (!tok.name().isNull())
		list += QRegExp::escape(tok.name());
    } else {
	list.reserve(filter.count());
	foreach(const PropToken t, filter) {
	    const CPropToken& tok = m_tokens.value(t);
	    if (!tok.name().isNull())
		list += QRegExp::escape(tok.name());
	}
    }
    return list;
}

const QList<PropToken> g_preproc = {
    TOK_CPP_IF,
    TOK_CPP_ELSE,
    TOK_CPP_ENDIF,
    TOK_CPP_IFDEF,
    TOK_CPP_IFNDEF,
    TOK_CPP_ELIF,
    TOK_CPP_ELIFDEF,
    TOK_CPP_ELIFNDEF,
    TOK_CPP_DEFINE,
    TOK_CPP_UNDEF,
};

const QList<PropToken> g_keywords = {
    TOK_INLINECCODE,
    TOK_BYTE,
    TOK_WORD,
    TOK_LONG,
    TOK_FVAR,
    TOK_FVARS,
    TOK_ASMCLK,

    TOK_INSTR,
    TOK_INSTRMODIFIER,
    TOK_HWREG,
    TOK_ORG,
    TOK_ORGH,
    TOK_ORGF,
    TOK_RES,
    TOK_FIT,
    TOK_ALIGNL,
    TOK_ALIGNW,

    TOK_REPEAT,
    TOK_FROM,
    TOK_TO,
    TOK_STEP,
    TOK_WHILE,
    TOK_UNTIL,
    TOK_IF,
    TOK_IFNOT,
    TOK_ELSE,
    TOK_ELSEIF,
    TOK_ELSEIFNOT,
    TOK_THEN,
    TOK_ENDIF,

    TOK_LOOKDOWN,
    TOK_LOOKDOWNZ,
    TOK_LOOKUP,
    TOK_LOOKUPZ,
    TOK_COGINIT,
    TOK_COGNEW,

    TOK_CASE,
    TOK_CASE_FAST,
    TOK_OTHER,

    TOK_QUIT,
    TOK_NEXT,

    TOK_ALLOCA,

    // other stuff
    TOK_ABORT,
    TOK_RESULT,
    TOK_RETURN,
    TOK_INDENT,
    TOK_OUTDENT,
    TOK_EOLN,
    TOK_EOF,
    TOK_DOTS,
    TOK_HERE,
    TOK_STRINGPTR,
    TOK_FILE,

    TOK_NOP,
    TOK_ROR,
    TOK_ROL,
    TOK_SHR,
    TOK_SHL,
    TOK_RCR,
    TOK_RCL,
    TOK_SAR,
    TOK_SAL,
    TOK_ADD,
    TOK_ADDX,
    TOK_ADDS,
    TOK_ADDSX,
    TOK_SUB,
    TOK_SUBX,
    TOK_SUBS,
    TOK_SUBSX,
    TOK_CMP,
    TOK_CMPX,
    TOK_CMPS,
    TOK_CMPSX,
    TOK_CMPR,
    TOK_CMPM,
    TOK_SUBR,
    TOK_CMPSUB,
    TOK_FGE,
    TOK_FLE,
    TOK_FGES,
    TOK_FLES,
    TOK_SUMC,
    TOK_SUMNC,
    TOK_SUMZ,
    TOK_SUMNZ,
    TOK_TESTB,
    TOK_TESTBN,
    TOK_BITL,
    TOK_BITH,
    TOK_BITC,
    TOK_BITNC,
    TOK_BITZ,
    TOK_BITNZ,
    TOK_BITRND,
    TOK_BITNOT,
    TOK_AND,
    TOK_ANDN,
    TOK_OR,
    TOK_XOR,
    TOK_MUXC,
    TOK_MUXNC,
    TOK_MUXZ,
    TOK_MUXNZ,
    TOK_MOV,
    TOK_NOT,
    TOK_ABS,
    TOK_NEG,
    TOK_NEGC,
    TOK_NEGNC,
    TOK_NEGZ,
    TOK_NEGNZ,
    TOK_INCMOD,
    TOK_DECMOD,
    TOK_ZEROX,
    TOK_SIGNX,
    TOK_ENCOD,
    TOK_ONES,
    TOK_TEST,
    TOK_TESTN,
    TOK_SETNIB,
    TOK_GETNIB,
    TOK_ROLNIB,
    TOK_SETBYTE,
    TOK_GETBYTE,
    TOK_ROLBYTE,
    TOK_SETWORD,
    TOK_GETWORD,
    TOK_ROLWORD,
    TOK_ALTSN,
    TOK_ALTGN,
    TOK_ALTSB,
    TOK_ALTGB,
    TOK_ALTSW,
    TOK_ALTGW,
    TOK_ALTR,
    TOK_ALTD,
    TOK_ALTS,
    TOK_ALTB,
    TOK_ALTI,
    TOK_SETR,
    TOK_SETD,
    TOK_SETS,
    TOK_DECOD,
    TOK_BMASK,
    TOK_CRCBIT,
    TOK_CRCNIB,
    TOK_MUXNITS,
    TOK_MUXNIBS,
    TOK_MUXQ,
    TOK_MOVBYTS,
    TOK_MUL,
    TOK_MULS,
    TOK_SCA,
    TOK_SCAS,
    TOK_ADDPIX,
    TOK_MULPIX,
    TOK_BLNPIX,
    TOK_MIXPIX,
    TOK_ADDCT1,
    TOK_ADDCT2,
    TOK_ADDCT3,
    TOK_WMLONG,
    TOK_RQPIN,
    TOK_RDPIN,
    TOK_RDLUT,
    TOK_RDBYTE,
    TOK_RDWORD,
    TOK_RDLONG,
    TOK_POPA,
    TOK_POPB,
    TOK_CALLD,
    TOK_RESI3,
    TOK_RESI2,
    TOK_RESI1,
    TOK_RESI0,
    TOK_RETI3,
    TOK_RETI2,
    TOK_RETI1,
    TOK_RETI0,
    TOK_CALLPA,
    TOK_CALLPB,
    TOK_DJZ,
    TOK_DJNZ,
    TOK_DJF,
    TOK_DJNF,
    TOK_IJZ,
    TOK_IJNZ,
    TOK_TJZ,
    TOK_TJNZ,
    TOK_TJF,
    TOK_TJNF,
    TOK_TJS,
    TOK_TJNS,
    TOK_TJV,
    TOK_JINT,
    TOK_JCT1,
    TOK_JCT2,
    TOK_JCT3,
    TOK_JSE1,
    TOK_JSE2,
    TOK_JSE3,
    TOK_JSE4,
    TOK_JPAT,
    TOK_JFBW,
    TOK_JXMT,
    TOK_JXFI,
    TOK_JXRO,
    TOK_JXRL,
    TOK_JATN,
    TOK_JQMT,
    TOK_JNINT,
    TOK_JNCT1,
    TOK_JNCT2,
    TOK_JNCT3,
    TOK_JNSE1,
    TOK_JNSE2,
    TOK_JNSE3,
    TOK_JNSE4,
    TOK_JNPAT,
    TOK_JNFBW,
    TOK_JNXMT,
    TOK_JNXFI,
    TOK_JNXRO,
    TOK_JNXRL,
    TOK_JNATN,
    TOK_JNQMT,
    TOK_SETPAT,
    TOK_AKPIN,
    TOK_WRPIN,
    TOK_WXPIN,
    TOK_WYPIN,
    TOK_WRLUT,
    TOK_WRBYTE,
    TOK_WRWORD,
    TOK_WRLONG,
    TOK_PUSHA,
    TOK_PUSHB,
    TOK_RDFAST,
    TOK_WRFAST,
    TOK_FBLOCK,
    TOK_XINIT,
    TOK_XSTOP,
    TOK_XZERO,
    TOK_XCONT,
    TOK_REP,
    TOK_COGINIT,
    TOK_QMUL,
    TOK_QDIV,
    TOK_QFRAC,
    TOK_QSQRT,
    TOK_QROTATE,
    TOK_QVECTOR,
    TOK_HUBSET,
    TOK_COGID,
    TOK_COGSTOP,
    TOK_LOCKNEW,
    TOK_LOCKRET,
    TOK_LOCKTRY,
    TOK_LOCKREL,
    TOK_QLOG,
    TOK_QEXP,
    TOK_RFBYTE,
    TOK_RFWORD,
    TOK_RFLONG,
    TOK_RFVAR,
    TOK_RFVARS,
    TOK_WFBYTE,
    TOK_WFWORD,
    TOK_WFLONG,
    TOK_GETQX,
    TOK_GETQY,
    TOK_GETCT,
    TOK_GETRND,
    TOK_SETDACS,
    TOK_SETXFRQ,
    TOK_FETXACC,
    TOK_WAITX,
    TOK_SETSE1,
    TOK_SETSE2,
    TOK_SETSE3,
    TOK_SETSE4,
    TOK_POLLINT,
    TOK_POLLCT1,
    TOK_POLLCT2,
    TOK_POLLCT3,
    TOK_POLLSE1,
    TOK_POLLSE2,
    TOK_POLLSE3,
    TOK_POLLSE4,
    TOK_POLLPAT,
    TOK_POLLFBW,
    TOK_POLLXMT,
    TOK_POLLXFI,
    TOK_POLLXRO,
    TOK_POLLXRL,
    TOK_POLLATN,
    TOK_POLLQMT,
    TOK_WAITINT,
    TOK_WAITCT1,
    TOK_WAITCT2,
    TOK_WAITCT3,
    TOK_WAITSE1,
    TOK_WAITSE2,
    TOK_WAITSE3,
    TOK_WAITSE4,
    TOK_WAITPAT,
    TOK_WAITFBW,
    TOK_WAITXMT,
    TOK_WAITXFI,
    TOK_WAITXRO,
    TOK_WAITXRL,
    TOK_WAITATN,
    TOK_ALLOWI,
    TOK_STALLI,
    TOK_TRGINT1,
    TOK_TRGINT2,
    TOK_TRGINT3,
    TOK_NIXINT1,
    TOK_NIXINT2,
    TOK_NIXINT3,
    TOK_SETINT1,
    TOK_SETINT2,
    TOK_SETINT3,
    TOK_SETQ,
    TOK_SETQ2,
    TOK_PUSH,
    TOK_POP,
    TOK_JMP,
    TOK_CALL,
    TOK_RET,
    TOK_CALLA,
    TOK_RETA,
    TOK_CALLB,
    TOK_RETB,
    TOK_JMPREL,
    TOK_SKIP,
    TOK_SKIPF,
    TOK_EXECF,
    TOK_GETPTR,
    TOK_GETBRK,
    TOK_COGBRK,
    TOK_BRK,
    TOK_SETLUTS,
    TOK_SETCY,
    TOK_SETCI,
    TOK_SETCQ,
    TOK_SETCFRQ,
    TOK_SETCMOD,
    TOK_SETPIV,
    TOK_SETPIX,
    TOK_COGATN,
    TOK_TESTP,
    TOK_TESTPN,
    TOK_DIRL,
    TOK_DIRH,
    TOK_DIRC,
    TOK_DIRNC,
    TOK_DIRZ,
    TOK_DIRNZ,
    TOK_DIRRND,
    TOK_DIRNOT,
    TOK_OUTL,
    TOK_OUTH,
    TOK_OUTC,
    TOK_OUTNC,
    TOK_OUTZ,
    TOK_OUTNZ,
    TOK_OUTRND,
    TOK_OUTNOT,
    TOK_FLTL,
    TOK_FLTH,
    TOK_FLTC,
    TOK_FLTNC,
    TOK_FLTZ,
    TOK_FLTNZ,
    TOK_FLTRND,
    TOK_FLTNOT,
    TOK_DRVL,
    TOK_DRVH,
    TOK_DRVC,
    TOK_DRVNC,
    TOK_DRVZ,
    TOK_DRVNZ,
    TOK_DRVRND,
    TOK_DRVNOT,
    TOK_SPLITB,
    TOK_MERGEB,
    TOK_SPLITW,
    TOK_MERGEW,
    TOK_SEUSSR,
    TOK_RGBSQZ,
    TOK_RGBEXP,
    TOK_XORO32,
    TOK_REV,
    TOK_RCZR,
    TOK_RCZL,
    TOK_WRC,
    TOK_RCNC,
    TOK_WRZ,
    TOK_RCNZ,
    TOK_MODCZ,
    TOK_MODC,
    TOK_MODZ,
    TOK_SETSCP,
    TOK_GETSCP,
    TOK_LOC,
    TOK_AUGS,
    TOK_AUGD,
    TOK__RET_,
};

const QList<PropToken> g_sections = {
    TOK_CON,
    TOK_VAR,
    TOK_DAT,
    TOK_PUB,
    TOK_PRI,
    TOK_OBJ,
    TOK_ASM,
};

const QList<PropToken> g_operator = {
    TOK_ASSIGN,
    TOK_OP_XOR,
    TOK_OP_OR,
    TOK_OP_AND,
    TOK_OP_ORELSE,
    TOK_OP_ANDTHEN,
    TOK_OP_GE,
    TOK_OP_LE,
    TOK_OP_GEU,
    TOK_OP_LEU,
    TOK_OP_GTU,
    TOK_OP_LTU,
    TOK_OP_NE,
    TOK_OP_EQ,
    TOK_OP_SGNCOMP,
    TOK_OP_LIMITMIN,
    TOK_OP_LIMITMAX,
    TOK_OP_REMAINDER,
    TOK_OP_UNSDIV,
    TOK_OP_UNSMOD,
    TOK_OP_FRAC,
    TOK_OP_HIGHMULT,
    TOK_OP_SCAS,
    TOK_OP_UNSHIGHMULT,
    TOK_OP_ROTR,
    TOK_OP_ROTL,
    TOK_OP_SHL,
    TOK_OP_SHR,
    TOK_OP_SAR,
    TOK_OP_REV,
    TOK_OP_REV2,
    TOK_OP_ADDBITS,
    TOK_OP_ADDPINS,
    TOK_OP_NEGATE,
    TOK_OP_BIT_NOT,
    TOK_OP_SQRT,
    TOK_OP_ABS,
    TOK_OP_DECODE,
    TOK_OP_ENCODE,
    TOK_OP_ENCODE2,
    TOK_OP_NOT,
    TOK_OP_DOUBLETILDE,
    TOK_OP_INCREMENT,
    TOK_OP_DECREMENT,
    TOK_OP_DOUBLEAT,
    TOK_OP_TRIPLEAT,
};

const CPropTokens g_tokens;
