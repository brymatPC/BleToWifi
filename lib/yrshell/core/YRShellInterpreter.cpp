
#include "YRShellInterpreter.h"
#include "compile/compiledDictionary.h"

uint8_t	YRShellInterpreter::s_shellNumber = 0;

/*
static uint16_t compiledDictionaryData[] = {
};
YRShellCompiledDictionary compiledDictionary( compiledDictionaryData, 0xFFFF , 0x0000 , YRSHELL_DICTIONARY_COMPILED);
*/

static const FunctionEntry interpreterFunctions[] = {
    { (uint16_t)YRShellInterpreter::SI_CC_return,     ""},
    { (uint16_t)YRShellInterpreter::SI_CC_space,      "space"},
    { (uint16_t)YRShellInterpreter::SI_CC_cr,         "cr"},
    { (uint16_t)YRShellInterpreter::SI_CC_lf,         "lf"},
    { (uint16_t)YRShellInterpreter::SI_CC_crlf,       "crlf"},
    { (uint16_t)YRShellInterpreter::SI_CC_reset,      "reset"},
    { (uint16_t)YRShellInterpreter::SI_CC_prompt,     "prompt"},
    { (uint16_t)YRShellInterpreter::SI_CC_clearPad,   "clearPad"},
    { (uint16_t)YRShellInterpreter::SI_CC_dot,        "."},
    { (uint16_t)YRShellInterpreter::SI_CC_dotByte,    ".b"},
    { (uint16_t)YRShellInterpreter::SI_CC_dotWord,    ".w"},
    { (uint16_t)YRShellInterpreter::SI_CC_dotX,       ".x"},
    { (uint16_t)YRShellInterpreter::SI_CC_dotByteX,   ".bx"},
    { (uint16_t)YRShellInterpreter::SI_CC_dotWordX,   ".wx"},
    { (uint16_t)YRShellInterpreter::SI_CC_dotStr,     ".str"},
    { (uint16_t)YRShellInterpreter::SI_CC_hex,        "hex"},
    { (uint16_t)YRShellInterpreter::SI_CC_decimal,    "decimal"},
    { (uint16_t)YRShellInterpreter::SI_CC_stringDef,  "s'"},
    { (uint16_t)YRShellInterpreter::SI_CC_if,         "["},
    { (uint16_t)YRShellInterpreter::SI_CC_else,       "]["},
    { (uint16_t)YRShellInterpreter::SI_CC_then,       "]"},
    { (uint16_t)YRShellInterpreter::SI_CC_begin,      "{"},
    { (uint16_t)YRShellInterpreter::SI_CC_until,      "}"},
    { (uint16_t)YRShellInterpreter::SI_CC_dup,        "dup"},
    { (uint16_t)YRShellInterpreter::SI_CC_swap,       "swap"},
    { (uint16_t)YRShellInterpreter::SI_CC_nip,        "nip"},
    { (uint16_t)YRShellInterpreter::SI_CC_tuck,       "tuck"},
    { (uint16_t)YRShellInterpreter::SI_CC_drop,       "drop"},
    { (uint16_t)YRShellInterpreter::SI_CC_rot,        "rot"},
    { (uint16_t)YRShellInterpreter::SI_CC_toR,        ">r"},
    { (uint16_t)YRShellInterpreter::SI_CC_fromR,      "r>"},
    { (uint16_t)YRShellInterpreter::SI_CC_negate,     "neg"},
    { (uint16_t)YRShellInterpreter::SI_CC_plus,       "+"},
    { (uint16_t)YRShellInterpreter::SI_CC_minus,      "-"},
    { (uint16_t)YRShellInterpreter::SI_CC_multiply,   "*"},
    { (uint16_t)YRShellInterpreter::SI_CC_divide,     "/"},
    { (uint16_t)YRShellInterpreter::SI_CC_mod,        "%"},
    { (uint16_t)YRShellInterpreter::SI_CC_and,        "&"},
    { (uint16_t)YRShellInterpreter::SI_CC_or,         "|"},
    { (uint16_t)YRShellInterpreter::SI_CC_xor,        "^"},
    { (uint16_t)YRShellInterpreter::SI_CC_lessThan,   "<"},
    { (uint16_t)YRShellInterpreter::SI_CC_lessEqual,  "<="},
    { (uint16_t)YRShellInterpreter::SI_CC_equal,      "=="},
    { (uint16_t)YRShellInterpreter::SI_CC_greaterThan,        ">"},
    { (uint16_t)YRShellInterpreter::SI_CC_greaterThanEqual,   ">="},
    { (uint16_t)YRShellInterpreter::SI_CC_setDebug,   "setDebug"},
    { (uint16_t)YRShellInterpreter::SI_CC_toC,        ">c"},
    { (uint16_t)YRShellInterpreter::SI_CC_fromC,      "c>"},
    { (uint16_t)YRShellInterpreter::SI_CC_parameterStackDepth,    "psd?"},
    { (uint16_t)YRShellInterpreter::SI_CC_returnStackDepth,       "rsd?"},
    { (uint16_t)YRShellInterpreter::SI_CC_compileStackDepth,      "csd?"},
    { (uint16_t)YRShellInterpreter::SI_CC_parameterStackAt,       "ps@" },
    { (uint16_t)YRShellInterpreter::SI_CC_returnStackAt,          "rs@" },
    { (uint16_t)YRShellInterpreter::SI_CC_compileStackAt,         "cs@" },
    { (uint16_t)YRShellInterpreter::SI_CC_notEqual,               "!=" },
    { (uint16_t)YRShellInterpreter::SI_CC_zeroEqual,              "0==" },
    { (uint16_t)YRShellInterpreter::SI_CC_zeroNotEqual,           "0!=" },
    { (uint16_t)YRShellInterpreter::SI_CC_over,                   "over" },
    { (uint16_t)YRShellInterpreter::SI_CC_2dup,                   "2dup" },
    { (uint16_t)YRShellInterpreter::SI_CC_2drop,                  "2drop" },
    { (uint16_t)YRShellInterpreter::SI_CC_strlen,                 "strlen" },
    { (uint16_t)YRShellInterpreter::SI_CC_getCurrentDictionary,                   "getCurrentDictionary" },
    { (uint16_t)YRShellInterpreter::SI_CC_getCurrentDictionaryEnd,                "getCurrentDictionaryEnd" },
    { (uint16_t)YRShellInterpreter::SI_CC_getCurrentDictionaryLastWord,           "getCurrentDictionaryLastWord" },
    { (uint16_t)YRShellInterpreter::SI_CC_delay,                                  "delay" },
    
    { (uint16_t)YRShellInterpreter::SI_CC_nextEntry,                              "nextEntry" },
    { (uint16_t)YRShellInterpreter::SI_CC_dotEntryName,                           ".entryName" },
    { (uint16_t)YRShellInterpreter::SI_CC_entryToken,                             "entryToken" },
    { (uint16_t)YRShellInterpreter::SI_CC_keyQ,                                   "key?" },
    { (uint16_t)YRShellInterpreter::SI_CC_auxKeyQ,                                "auxKey?" },
    { (uint16_t)YRShellInterpreter::SI_CC_auxIO,                                  "auxIO" },
    { (uint16_t)YRShellInterpreter::SI_CC_mainIO,                                 "mainIO" },
    { (uint16_t)YRShellInterpreter::SI_CC_emit,                                   "emit" },
    { (uint16_t)YRShellInterpreter::SI_CC_auxEmit,                                "auxEmit" },

    { (uint16_t)YRShellInterpreter::SI_CC_shellSize,                              "shellSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_printShellClass,                        "printShellClass" },
    
    { (uint16_t)YRShellInterpreter::SI_CC_dictionarySize,                        "dictionarySize" },
    { (uint16_t)YRShellInterpreter::SI_CC_padSize,                               "padSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_numRegisters,                          "numRegisters" },
    { (uint16_t)YRShellInterpreter::SI_CC_parameterStackSize,                    "parameterStackSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_returnStackSize,                       "returnStackSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_compileStackSize,                      "compileStackSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_inqSize,                               "inqSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_auxInqSize,                            "auxInqSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_outqSize,                              "outqSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_auxOutqSize,                           "auxOutqSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_dictionaryClear,                       "dictClear" },
    { (uint16_t)YRShellInterpreter::SI_CC_setCommandEcho,                        "setCommandEcho" },
    { (uint16_t)YRShellInterpreter::SI_CC_setExpandCR,                           "setExpandCR" },
    { (uint16_t)YRShellInterpreter::SI_CC_systicks,                              "systicks" },
    { (uint16_t)YRShellInterpreter::SI_CC_micros,                                "micros" },
    { (uint16_t)YRShellInterpreter::SI_CC_millis,                                "millis" },

    { (uint16_t)YRShellInterpreter::SI_CC_bang,                                	 "!" },
    { (uint16_t)YRShellInterpreter::SI_CC_at,                                    "@" },
    { (uint16_t)YRShellInterpreter::SI_CC_clearStats,                            "clearStats" },
    { (uint16_t)YRShellInterpreter::SI_CC_sliceStats,                            "sliceStats" },
    { (uint16_t)YRShellInterpreter::SI_CC_printSliceName,                        "printSliceName" },
    
    { (uint16_t)YRShellInterpreter::SI_CC_find,                                  "find" },
    { (uint16_t)YRShellInterpreter::SI_CC_findEntry,                             "findEntry" },
    { (uint16_t)YRShellInterpreter::SI_CC_fetchToken,                            "fetchToken" },

    { (uint16_t)YRShellInterpreter::SI_CC_lshift,                                "<<" },
    { (uint16_t)YRShellInterpreter::SI_CC_irshift,                               "i>>" },
    { (uint16_t)YRShellInterpreter::SI_CC_rshift,                                ">>" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_return,                              "v_return" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_uint16,                              "v_uint16" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_uint32,                              "v_uint32" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_nint16,                              "v_nint16" },

    { (uint16_t)YRShellInterpreter::SI_CC_noop,                                  "noop" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_if,                                  "v_if" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_else,                                "v_else" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_then,                                "v_then" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_begin,                               "v_begin" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_until,                               "v_until" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_string,                              "v_string" },
    { (uint16_t)YRShellInterpreter::SI_CC_isFunction,                            "isFunction" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_dictionaryMask,                      "v_dictionaryMask" },
    { (uint16_t)YRShellInterpreter::SI_CC_v_dictionaryRelative,                  "v_dictionaryRelative" },

    { (uint16_t)YRShellInterpreter::SI_CC_not,                                   "~" },
    { (uint16_t)YRShellInterpreter::SI_CC_dotRawStr,                             ".rawStr" },
    { (uint16_t)YRShellInterpreter::SI_CC_dotN,                                  ".n" },

    { (uint16_t)YRShellInterpreter::SI_CC_setBaud,                                "setBaud" },
    { (uint16_t)YRShellInterpreter::SI_CC_sysTicksPerSecond,                      "sysTicksPerSecond" },

    { (uint16_t)YRShellInterpreter::SI_CC_lastBufferSize,                         "lastBufferSize" },
    { (uint16_t)YRShellInterpreter::SI_CC_printMainFileName,                      "printMainFileName" },

    { (uint16_t)YRShellInterpreter::SI_CC_setPromptEnable,                        "setPromptEnable" },

    { (uint16_t)YRShellInterpreter::SI_CC_strBang,                                "str!" },
    { (uint16_t)YRShellInterpreter::SI_CC_charBang,                               "c!" },
    { (uint16_t)YRShellInterpreter::SI_CC_charAt,                                 "c@" },
    { (uint16_t)YRShellInterpreter::SI_CC_textIO,                                 "textIO" },

    { (uint16_t)YRShellInterpreter::SI_CC_nextDelay,                              "nextDelay" },
    
    { (uint16_t)YRShellInterpreter::SI_CC_isEntryMatch,                           "isEntryMatch" },


#ifdef YRSHELL_INTERPRETER_FLOATING_POINT
    { (uint16_t)YRShellInterpreter::SI_CC_dotf,                                   ".f" },
    { (uint16_t)YRShellInterpreter::SI_CC_dote,                                   ".e" },
    { (uint16_t)YRShellInterpreter::SI_CC_fLessThan,                              "f<" },
    { (uint16_t)YRShellInterpreter::SI_CC_fLessEqual,                             "f<="},
    { (uint16_t)YRShellInterpreter::SI_CC_fEqual,                                 "f=="},
    { (uint16_t)YRShellInterpreter::SI_CC_fGreaterThan,                           "f>"},
    { (uint16_t)YRShellInterpreter::SI_CC_fGreaterThanEqual,                      "f>="},
    
    { (uint16_t)YRShellInterpreter::SI_CC_fPlus,                                  "f+" },
    { (uint16_t)YRShellInterpreter::SI_CC_fMinus,                                 "f-" },
    { (uint16_t)YRShellInterpreter::SI_CC_fMultiply,                              "f*" },
    { (uint16_t)YRShellInterpreter::SI_CC_fDivide,                                "f/" },
    { (uint16_t)YRShellInterpreter::SI_CC_fPI,                                    "fPI" },
    { (uint16_t)YRShellInterpreter::SI_CC_fSin,                                   "fsin" },
    { (uint16_t)YRShellInterpreter::SI_CC_fCos,                                   "fcos" },
    { (uint16_t)YRShellInterpreter::SI_CC_fTan,                                   "ftan" },
    { (uint16_t)YRShellInterpreter::SI_CC_faSin,                                  "fasin" },
    { (uint16_t)YRShellInterpreter::SI_CC_faCos,                                  "facos" },
    { (uint16_t)YRShellInterpreter::SI_CC_faTan,                                  "fatan" },
    { (uint16_t)YRShellInterpreter::SI_CC_fExp,                                   "fexp" },
    { (uint16_t)YRShellInterpreter::SI_CC_fLog,                                   "flog" },
    { (uint16_t)YRShellInterpreter::SI_CC_fPow,                                   "fpow" },
    { (uint16_t)YRShellInterpreter::SI_CC_fLog10,                                 "flog10" },
    { (uint16_t)YRShellInterpreter::SI_CC_fRound,                                 "fround" },
    { (uint16_t)YRShellInterpreter::SI_CC_floatToInt,                             "f>i" },
    { (uint16_t)YRShellInterpreter::SI_CC_intToFloat,                             "i>f" },
 
#endif
    { (uint16_t)0, NULL}
};




static FunctionDictionary dictionaryInterpreterFunction( interpreterFunctions, YRSHELL_DICTIONARY_INTERPRETER_FUNCTION );

#ifdef YRSHELL_DEBUG
const char *stateDebugStrings[] = {
    "YRSHELL_INVALID_STATE",
	"YRSHELL_NOT_INITIALIZED",
    "YRSHELL_BEGINNING",
    "YRSHELL_INRESET",
    "YRSHELL_BEGIN_IDLE",
    "YRSHELL_IDLE",
    "YRSHELL_FILLING_PAD",
    "YRSHELL_FILLING_AUXPAD",
    "YRSHELL_BEGIN_PARSING",
    "YRSHELL_PARSING",
    "YRSHELL_COMPILING",
    "YRSHELL_EXECUTING",
    "YRSHELL_WAIT_FOR_OUTPUT_SPACE",
    "YRSHELL_WAIT_DELAY",
    "YRSHELL_OUTPUT",
    "YRSHELL_OUTPUT_STR",
    "YRSHELL_LAST_INVALID_STATE"
};
const char *SIDebugStrings[] = {
    "SI_CC_first",
    "SI_CC_return",
    "SI_CC_space",
    "SI_CC_cr",
    "SI_CC_lf",
    "SI_CC_crlf",
    "SI_CC_reset",
    "SI_CC_prompt",
    "SI_CC_clearPad",
    "SI_CC_uint16",
    "SI_CC_uint32",
    "SI_CC_nint16",
    "SI_CC_dot",
    "SI_CC_dotByte",
    "SI_CC_dotWord",
    "SI_CC_dotX",
    "SI_CC_dotByteX",
    "SI_CC_dotWordX",
    "SI_CC_dotStr",
    "SI_CC_hex",
    "SI_CC_decimal",
    "SI_CC_stringDef",
    "SI_CC_string",
    "SI_CC_begin",
    "SI_CC_else",
    "SI_CC_then",
    "SI_CC_begin",
    "SI_CC_until",
    "SI_CC_jmp",
    "SI_CC_jmpz",
    "SI_CC_dup",
    "SI_CC_swap",
    "SI_CC_nip",
    "SI_CC_tuck",
    "SI_CC_drop",
    "SI_CC_rot",
    "SI_CC_toR",
    "SI_CC_fromR",
    "SI_CC_negate",
    "SI_CC_plus",
    "SI_CC_minus",
    "SI_CC_multiply",
    "SI_CC_divide",
    "SI_CC_mod",
    "SI_CC_and",
    "SI_CC_or",
    "SI_CC_xor",
    "SI_CC_lessThan",
    "SI_CC_lessEqual",
    "SI_CC_equal",
    "SI_CC_greaterThan",
    "SI_CC_greaterThanEqual",
    "SI_CC_setDebug",
    "SI_CC_toC",
    "SI_CC_fromC",
    "SI_CC_parameterStackDepth",
    "SI_CC_returnStackDepth",
    "SI_CC_compileStackDepth",
    "SI_CC_parameterStackAt",
    "SI_CC_returnStackAt",
    "SI_CC_compileStackAt",
    "SI_CC_notEqual",
    "SI_CC_zeroEqual",
    "SI_CC_zeroNotEqual",
    "SI_CC_over",
    "SI_CC_2dup",
    "SI_CC_2drop",
    "SI_CC_strlen",
    "SI_CC_getCurrentDictionary",
    "SI_CC_getCurrentDictionaryEnd",
    "SI_CC_getCurrentDictionaryLastWord",
    "SI_CC_delay",
    "SI_CC_nextEntry",
    "SI_CC_dotEntryName",
    "SI_CC_entryToken",
    "SI_CC_keyQ",
    "SI_CC_auxKeyQ",
    "SI_CC_auxIO",
    "SI_CC_mainIO",
    "SI_CC_emit",
    "SI_CC_auxEmit",

    "SI_CC_shellSize",
    "SI_CC_printShellClass",

    "SI_CC_dictionarySize",
    "SI_CC_padSize",
    "SI_CC_numRegisters",
    "SI_CC_parameterStackSize",
    "SI_CC_returnStackSize",
    "SI_CC_compileStackSize",
    "SI_CC_inqSize",
    "SI_CC_auxInqSize",
    "SI_CC_outqSize",
    "SI_CC_auxOutqSize",
 
    "SI_CC_dictionaryClear",
    "SI_CC_setCommandEcho",
    "SI_CC_setExpandCR",

    "SI_CC_systicks",
    "SI_CC_micros",
    "SI_CC_millis"

   	"SI_CC_bang",
    "SI_CC_at",

    "SI_CC_clearStats",
    "SI_CC_sliceStats",
    "SI_CC_printSliceName",

    "SI_CC_find",
    "SI_CC_findEntry",
    "SI_CC_fetchToken",
    
    "SI_CC_lshift",
    "SI_CC_irshift",
    "SI_CC_rshift",
    "SI_CC_v_return",
    "SI_CC_v_uint16",
    "SI_CC_v_uint32",
    "SI_CC_v_nint16",
 
    "SI_CC_noop",
    "SI_CC_x_if",
    "SI_CC_x_else",
    "SI_CC_x_then",
    "SI_CC_x_begin",
    "SI_CC_x_until",
    "SI_CC_v_if",
    "SI_CC_v_else",
    "SI_CC_v_then",
    "SI_CC_v_begin",
    "SI_CC_v_until",
    "SI_CC_v_string",
    "SI_CC_isFunction",
    "SI_CC_v_dictionaryMask",
    "SI_CC_v_dictionaryRelative",
    "SI_CC_not",
    
    "SI_CC_dotRawStr",
    "SI_CC_dotN",

    "SI_CC_setBaud",
    "SI_CC_sysTicksPerSecond",

    "SI_CC_lastBufferSize",
    "SI_CC_printMainFileName",
    
    "SI_CC_setPromptEnable",
    
    "SI_CC_strBang",
    "SI_CC_charBang",
    "SI_CC_charAt",
    "SI_CC_textIO",

    "SI_CC_nextDelay",
    
    "SI_CC_isEntryMatch",

#ifdef YRSHELL_INTERPRETER_FLOATING_POINT
    "SI_CC_dotf",
    "SI_CC_dote",
    "SI_CC_fLessThan",
    "SI_CC_fLessEqual",
    "SI_CC_fEqual",
    "SI_CC_fGreaterThan",
    "SI_CC_fGreaterThanEqual",

    "SI_CC_fPlus",
    "SI_CC_fMinus",
    "SI_CC_fMultiply",
    "SI_CC_fDivide",
    "SI_CC_fPI",
    "SI_CC_fSin",
    "SI_CC_fCos",
    "SI_CC_fTan",
    "SI_CC_faSin",
    "SI_CC_faCos",
    "SI_CC_faTan",
    "SI_CC_fExp",
    "SI_CC_fLog",
    "SI_CC_fPow",
    "SI_CC_fLog10",
    "SI_CC_fRound",
    "SI_CC_floatToInt",
    "SI_CC_intToFloat",
#endif
    
    "SI_CC_last"
};
#endif

static char charToHex( char c) {
    char value = '\0';
    if(  c >= '0' && c <= '9' ) {
        value |= c - '0';
    } else if(  c >= 'a' && c <= 'f' ) {
        value |= c - 'a' + 10;
    } else if(  c >= 'A' && c <= 'F') {
        value |= c - 'A' + 10;
    }
    return value;
}
static bool stringToUnsignedX( const char* P, uint32_t* V){
    bool rc = false;
    uint32_t value = 0, numDigits = 0;
    if( (*P != '0') || (*(P+1) != 'x')) {
        rc = false;
    } else {
        P +=2;
        while( ((*P >= '0' && *P <= '9') ||  (*P >= 'a' && *P <= 'f')  ||  (*P >= 'A' && *P <= 'F'))  && numDigits++ < 16 ) {
            value <<= 4;
            value |= charToHex( *P++);
        }
        if( *P == '\0') {
            rc = true;
        }
        *V = value;
    }
    return rc;
    
}
static void unsignedToStringZero(uint32_t num, uint8_t numDigits, char *s) {
    char *P = s + numDigits;
    *P-- = '\0';
    while( P >= s) {
        *P-- = '0' + (num % 10);
        num /= 10;
    }
}
static void unsignedToString(uint32_t num, uint8_t numDigits, char *s) {
    unsignedToStringZero( num, numDigits, s);
    while( *s == '0' && numDigits-- > 1) {
        *s++ = ' ';
    }
}
static void unsignedToStringX(uint32_t num, uint8_t numDigits, char *s){
    *s++ = '0';
    *s++ = 'x';
    char *P = s + numDigits;
    char c;
    *P-- = '\0';
    while( P >= s) {
        c = num & 0xF;
        if( c > 9) {
            c += '7';
        } else {
            c += '0';
        }
        *P-- =  c;
        num >>= 4;
    }
}
static void intToString(int32_t num, uint8_t numDigits, char *s) {
    *s++ = ' ';
    if( num < 0) {
        unsignedToString( (unsigned) (0 - num), numDigits, s);
        while( *s == ' ') {
            s++;
        }
        s--;
        *s = '-';
    } else {
        unsignedToString( (unsigned) num, numDigits, s);
    }
}
static const char* stringToUnsignedInternal( const char* P, uint32_t* V) {
    const char* rc = P;
    bool negative = false;
    uint32_t value = 0, numDigits = 0;
    if( *P == '-') {
        negative = -1;
        P++;
    }
    while( *P >= '0' && *P <= '9' && numDigits++ < 16 ) {
        value *= 10;
        value += *P - '0';
        P++;
    }
    rc = P;
    if( negative) {
        *V = 0-value;
    } else {
        *V = value;
    }
    return rc;
}
static bool stringToUnsigned( const char* P, uint32_t* V){
    bool rc = false;
    if( *P == '\0') {
        rc = 0;
    } else {
        P = stringToUnsignedInternal(P, V);
        if( *P == '\0') {
            rc = true;
        }
    }
    return rc;
}
CompiledDictionary emptyDictionary;

void YRShellInterpreter::init( ) {
    reset();
    
    m_dictionaryList[ YRSHELL_DICTIONARY_DUMMY_INDEX] = &emptyDictionary;
    m_dictionaryList[ YRSHELL_DICTIONARY_CURRENT_INDEX] = m_DictionaryCurrent;
    m_dictionaryList[ YRSHELL_DICTIONARY_EXTENSION_COMPILED_INDEX] = NULL;
    m_dictionaryList[ YRSHELL_DICTIONARY_EXTENSION_FUNCTION_INDEX] = NULL;
    m_dictionaryList[ YRSHELL_DICTIONARY_INTERPRETER_COMPILED_INDEX] = (Dictionary*)&interpreterCompiledDictionary;
    m_dictionaryList[ YRSHELL_DICTIONARY_COMMON_FUNCTION_INDEX] = NULL;
    m_dictionaryList[ YRSHELL_DICTIONARY_INTERPRETER_FUNCTION_INDEX] =  &dictionaryInterpreterFunction;
}
YRShellInterpreter::YRShellInterpreter() {
    m_DictionaryCurrent = NULL;
    m_Inq = m_AuxInq = m_Outq = m_AuxOutq = NULL;
    m_ParameterStack = m_ReturnStack = m_CompileStack = NULL;
    m_parameterStackSize = m_returnStackSize = m_compileStackSize = 0;
    m_commandEcho = m_expandCR = true;

    m_Pad = NULL;
    m_Registers = NULL;
    m_numRegisters = 0;
	m_compileTopOfStack = 0;
#ifdef YRSHELL_DEBUG
	m_debugFlags = 0;
#endif
	m_hexMode = false;
	m_outputTimeoutInMilliseconds = 1000;
	m_padCount = 0;
    m_padSize = 0;
	m_PC = YRSHELL_DICTIONARY_INVALID;
	m_returnTopOfStack = 0;
	m_saveptr = NULL;
	m_state = YRSHELL_NOT_INITIALIZED;
	m_token = NULL;
	m_topOfStack = 0;
    m_requestUseAuxQueues = false;
    m_requestUseMainQueues = false;
    m_lastUseAuxQueues = false;
	m_useAuxQueues = false;
    m_useTextOutput = false;
#ifdef  INPUT_BUFFER_EDITING
    m_LastBuffer = NULL;
    if( m_lastBufferSize != 0) {
        memset(m_LastBuffer, 0, m_lastBufferSize);
        m_lastBufferFree = 0;
        m_lastBufferIndex = 0;
    }
#endif
	strcpy( m_autoPrompt, "\rYR_");
	unsignedToStringZero(s_shellNumber++, 2, &m_autoPrompt[3]);
	strcat( m_autoPrompt, ">");
	setPrompt( m_autoPrompt);
    m_outputStrPtr  = NULL;
    m_outputUint = 0;
    m_outputStr0 = NULL;
    m_outputStr1 = NULL;
    m_outputStr2 = NULL;
    m_outputStr3 = NULL;
    m_stateTopOfStack = 0;
    m_promptEnable = true;
}
YRShellInterpreter::~YRShellInterpreter() {
}
bool YRShellInterpreter::isCompileToken() {
    return m_token != NULL && (!strcmp( m_token, "s'") || !strcmp( m_token, "[") || !strcmp( m_token, "][") || !strcmp( m_token, "]") || !strcmp( m_token, "{") || !strcmp( m_token, "}") );
}
uint16_t YRShellInterpreter::find( const char* name) {
    uint16_t rc = YRSHELL_DICTIONARY_INVALID;
    for( uint8_t i = 0; rc == YRSHELL_DICTIONARY_INVALID && i < YRSHELL_DICTIONARY_LAST_INDEX; i++) {
        if( m_dictionaryList[ i] != NULL) {
            rc = m_dictionaryList[ i]->find( name);
        }
    }
    return rc;
}
uint16_t YRShellInterpreter::findEntry( const char* name) {
    uint16_t rc = YRSHELL_DICTIONARY_INVALID;
    for( uint8_t i = 0; rc == YRSHELL_DICTIONARY_INVALID && i < YRSHELL_DICTIONARY_LAST_INDEX; i++) {
        if( m_dictionaryList[ i] != NULL) {
            rc = m_dictionaryList[ i]->findEntry( name);
        }
    }
    return rc;
}
const char* YRShellInterpreter::getFileName( const char* P) {
    const char* R = P;
    while( *P++ != '\0') {
        if( *P == '/' || *P == '\'') {
            R = ++P;
        }
    }
    return R;
}

void YRShellInterpreter::shellERROR( const char* file, unsigned line) {
    reset( file, line);
}
void YRShellInterpreter::shellERROR( const char* file, unsigned line, const char* message) {
    reset( file, line, message);
}
void YRShellInterpreter::CC_nextEntry( ) {
    uint32_t v1 = popParameterStack();
    uint32_t v2 = popParameterStack();
    uint32_t res = YRSHELL_DICTIONARY_INVALID;
    if( v1 == YRSHELL_DICTIONARY_INVALID && v2 == 0) {
        if( m_dictionaryList[ v2] != NULL) {
            res = m_dictionaryList[ v2]->getFirstEntry();
        }
    } else {
        if( v1 != YRSHELL_DICTIONARY_INVALID && v2 < YRSHELL_DICTIONARY_LAST_INDEX && m_dictionaryList[ v2] != NULL) {
            res = m_dictionaryList[ v2]->getNextEntry(v1);
        }
    }
    if( res == YRSHELL_DICTIONARY_INVALID) {
        v2++;
    }
    while( res == YRSHELL_DICTIONARY_INVALID && v2 < YRSHELL_DICTIONARY_LAST_INDEX ) {
        if(m_dictionaryList[ v2] != NULL ) {
            res = m_dictionaryList[ v2]->getFirstEntry();
            if( res == YRSHELL_DICTIONARY_INVALID ) {
                v2++;
            }
        } else {
            v2++;
        }
    }
    if( v2 > YRSHELL_DICTIONARY_LAST_INDEX) {
        v2 = YRSHELL_DICTIONARY_LAST_INDEX;
    }
    pushParameterStack( v2);
    pushParameterStack( res);
}

void YRShellInterpreter:: prompt( ) {
	if( m_promptEnable) {
		outString( m_prompt);
	}
}
void YRShellInterpreter::executeFunction( uint16_t n) {
    bool b;
    int32_t i;
    uint32_t v1, v2, v3;
    float f1, f2;
    const char *P, *M;
    Sliceable *S;

#ifdef YRSHELL_DEBUG
    if( m_debugFlags & YRSHELL_DEBUG_EXECUTE) {
        outString("[");
        if(n >= SI_CC_first && n <= SI_CC_last) {
            outString( SIDebugStrings[n - SI_CC_first]);
        }
        outString("]");
    }
#endif
    switch( n) {
        case SI_CC_return:
            m_PC = popReturnStack();
            break;
        case SI_CC_space:
            outChar( ' ');
            break;
        case SI_CC_cr:
            outChar( '\r');
            break;
        case SI_CC_prompt:
            prompt();
            break;
        case SI_CC_lf:
            outChar( '\n');
            break;
        case SI_CC_crlf:
            outChar( '\r');
            outChar( '\n');
            break;
        case SI_CC_reset:
            m_DictionaryCurrent->reset();
            reset( __FILE__, __LINE__, "COMMAND RESET");
            break;
        case SI_CC_uint16:
            v1 = fetchCurrentValueToken();
            pushParameterStack( v1);
            break;
        case SI_CC_uint32:
            v1 = fetchCurrentValueToken();
            v1 |= ((uint32_t)fetchCurrentValueToken()) << 16;
            pushParameterStack( v1);
            break;
        case SI_CC_nint16:
            v1 = fetchCurrentValueToken();
            v1 |= 0xFFFF0000;
            pushParameterStack( v1);
            break;
        case SI_CC_dot:
            if( m_hexMode) {
                outUint32X( popParameterStack());
            } else {
                outInt32( popParameterStack());
            }
            outChar( ' ');
            break;
        case SI_CC_dotByte:
            if( m_hexMode) {
                outUint8X( popParameterStack());
            } else {
                outInt8( popParameterStack());
            }
            outChar( ' ');
            break;
        case SI_CC_dotWord:
            if( m_hexMode) {
                outUint16X( popParameterStack());
            } else {
                outInt16( popParameterStack());
            }
            outChar( ' ');
            break;
        case SI_CC_dotX:
            outUint32X( popParameterStack());
            outChar( ' ');
            break;
        case SI_CC_dotByteX:
            outUint8X( popParameterStack());
            outChar( ' ');
            break;
        case SI_CC_dotWordX:
            outUint16X( popParameterStack());
            outChar( ' ');
            break;
        case SI_CC_dotStr:
            m_outputStrPtr = getAddressFromToken( popParameterStack());
            pushState(YRSHELL_OUTPUT_STR);
            break;
        case SI_CC_hex:
            m_hexMode = true;
            break;
        case SI_CC_decimal:
            m_hexMode = false;
            break;
        case SI_CC_stringDef:
            CC_stringDef();
            break;
        case SI_CC_string:
            pushParameterStack( getAbsoluteAddressToken( ));
            P = getAddressFromToken( m_PC);
            m_PC += Dictionary::nameLength(P);
            break;
        case SI_CC_if:
            CC_if();
            break;
        case SI_CC_else:
            CC_else();
            break;
        case SI_CC_then:
            CC_then();
            break;
        case SI_CC_begin:
            CC_begin();
            break;
        case SI_CC_until:
            CC_until();
            break;
        case SI_CC_x_else:
        case SI_CC_jmp:
            v1 = fetchCurrentToken();
            m_PC &= YRSHELL_DICTIONARY_RELATIVE_ALL_MASK;
            m_PC |= v1 & ~YRSHELL_DICTIONARY_RELATIVE_ALL_MASK;
            break;
        case SI_CC_x_if:
        case SI_CC_x_until:
        case SI_CC_jmpz:
            if( !popParameterStack()) {
                v1 = fetchCurrentToken();
                m_PC &= YRSHELL_DICTIONARY_RELATIVE_ALL_MASK;
                m_PC |= v1 & ~YRSHELL_DICTIONARY_RELATIVE_ALL_MASK;
            } else {
                m_PC++;
            }
            break;
        case SI_CC_dup:
            v1 = popParameterStack();
            pushParameterStack(v1);
            pushParameterStack(v1);
            break;
        case SI_CC_swap:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(v1);
            pushParameterStack(v2);
            break;
        case SI_CC_nip:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(v1);
            break;
        case SI_CC_tuck:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(v1);
            pushParameterStack(v2);
            pushParameterStack(v1);
            break;
        case SI_CC_drop:
            v1 = popParameterStack();
            break;
        case SI_CC_rot:
            v1 = popParameterStack();
            v2 = popParameterStack();
            v3 = popParameterStack();
            pushParameterStack(v2);
            pushParameterStack(v1);
            pushParameterStack(v3);
            break;
        case SI_CC_toR:
            v1 = popParameterStack();
            pushReturnStack(v1);
            break;
        case SI_CC_fromR:
            v1 = popReturnStack();
            pushParameterStack(v1);
            break;
        case SI_CC_negate:
            v1 = popParameterStack();
            pushParameterStack( 0 - v1);
            break;
        case SI_CC_plus:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v1 + v2);
            break;
        case SI_CC_minus:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v2 - v1);
            break;
        case SI_CC_multiply:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v1 * v2);
            break;
        case SI_CC_divide:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v2 / v1);
            break;
        case SI_CC_mod:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v2 % v1);
            break;
        case SI_CC_and:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v1 & v2);
            break;
        case SI_CC_or:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v1 | v2);
            break;
        case SI_CC_xor:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v1 ^ v2);
            break;
        case SI_CC_lessThan:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( ((int32_t) v2) < ((int32_t) v1));
            break;
        case SI_CC_lessEqual:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(  ((int32_t) v2)  <= ((int32_t) v1));
            break;
        case SI_CC_equal:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v2 == v1);
            break;
        case SI_CC_greaterThan:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(  ((int32_t) v2)  > ((int32_t) v1));
            break;
        case SI_CC_greaterThanEqual:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(  ((int32_t) v2)  >= ((int32_t) v1));
            break;
        case SI_CC_setDebug:
            v1 = popParameterStack();
#ifdef YRSHELL_DEBUG
            m_debugFlags = v1;
#endif
            break;
        case SI_CC_toC:
            v1 = popParameterStack();
            pushCompileStack(v1);
            break;
        case SI_CC_fromC:
            v1 = popCompileStack();
            pushParameterStack(v1);
            break;
        case SI_CC_parameterStackDepth:
            pushParameterStack( m_topOfStack);
            break;
        case SI_CC_returnStackDepth:
            pushParameterStack( m_returnTopOfStack);
            break;
        case SI_CC_compileStackDepth:
            pushParameterStack( m_compileTopOfStack);
            break;
        case SI_CC_parameterStackAt:
            v1 = m_topOfStack - 2 - popParameterStack();
            if( v1 >= m_topOfStack) {
                shellERROR( __FILE__, __LINE__, "BAD STACK ACCESS");
            } else {
                pushParameterStack( m_ParameterStack[ v1]);
            }
            break;
            
        case SI_CC_returnStackAt:
            v1 = m_returnTopOfStack - 1 - popParameterStack();
            if( v1 >= m_returnTopOfStack) {
                shellERROR( __FILE__, __LINE__, "BAD RETURN STACK ACCESS");
            } else {
                pushParameterStack( m_ReturnStack[ v1]);
            }
            break;
        case SI_CC_compileStackAt:
            v1 = m_compileTopOfStack - 1 - popParameterStack();
            if( v1 >= m_compileTopOfStack) {
                shellERROR( __FILE__, __LINE__, "BAD COMPILE STACK ACCESS");
            } else {
                pushParameterStack( m_CompileStack[ v1]);
            }
            break;
        case SI_CC_notEqual:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack( v2 != v1);
            break;
        case SI_CC_zeroEqual:
            v1 = popParameterStack();
            pushParameterStack( 0 == v1);
            break;
        case SI_CC_zeroNotEqual:
            v1 = popParameterStack();
            pushParameterStack( 0 != v1);
            break;
        case SI_CC_over:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(v2);
            pushParameterStack(v1);
            pushParameterStack(v2);
            break;
        case SI_CC_2dup:
            v1 = popParameterStack();
            v2 = popParameterStack();
            pushParameterStack(v2);
            pushParameterStack(v1);
            pushParameterStack(v2);
            pushParameterStack(v1);
            break;
        case SI_CC_2drop:
            v1 = popParameterStack();
            v2 = popParameterStack();
            break;
        case SI_CC_strlen:
            P = getAddressFromToken(popParameterStack());
            if( P == NULL) {
                pushParameterStack(0);
            } else {
                pushParameterStack((uint32_t) strlen(P));
            }
            break;
        case SI_CC_getCurrentDictionary:
            pushParameterStack( m_DictionaryCurrent->getWord(popParameterStack()));
            break;
        case SI_CC_getCurrentDictionaryEnd:
            pushParameterStack(m_DictionaryCurrent->getBackupWordEnd());
            break;
        case SI_CC_getCurrentDictionaryLastWord:
            pushParameterStack(m_DictionaryCurrent->getBackupLastWord());
            break;
            
        case SI_CC_delay:
            m_delayTimer.setInterval(popParameterStack());
            pushState(YRSHELL_WAIT_DELAY);
            break;
		case SI_CC_nextDelay:
			m_delayTimer.isNextInterval();
            pushState(YRSHELL_WAIT_DELAY);
			break;
       	case SI_CC_nextEntry:
            CC_nextEntry( );
            break;
        case SI_CC_dotEntryName:
            v1 = popParameterStack();
            v2 = popParameterStack();
            if( v1 != YRSHELL_DICTIONARY_INVALID && v2 < YRSHELL_DICTIONARY_LAST_INDEX && m_dictionaryList[ v2] != NULL) {
                v1 = m_dictionaryList[ v2]->getNameAddressToken(v1);
                P = getAddressFromToken( v1);
                if( P == NULL || strlen(P) == 0) {
                    pushParameterStack( 0);
                } else {
                    pushParameterStack((uint32_t) strlen( P));
                    m_outputStrPtr = P;
                    pushState( YRSHELL_OUTPUT_STR);
                }
            } else {
                pushParameterStack(0);
            }
            break;
        case SI_CC_isEntryMatch:
            v1 = popParameterStack();
            v2 = popParameterStack();
            v3 = popParameterStack();
            pushParameterStack(v3);
            pushParameterStack(v2);
            pushParameterStack(v1);
            if( v1 != YRSHELL_DICTIONARY_INVALID && v2 < YRSHELL_DICTIONARY_LAST_INDEX && m_dictionaryList[ v2] != NULL) {
                v1 = m_dictionaryList[ v2]->getNameAddressToken(v1);
                P = getAddressFromToken( v1);
                M = getAddressFromToken( v3);
                v1 = (uint32_t) strlen( P);
                v2 = (uint32_t) strlen( M);
                if( P == NULL || v1 == 0 || M == NULL || v2 == 0 || v1 < v2) {
                    pushParameterStack( 0);
                } else {
                    v3 = v1 - v2;
                    v1 = 0;
                    for( i = 0; i <= (int) v3; i++) {
                        if( !strncmp(P+i, M, v2) ) {
                            v1 = -1;
                            break;
                        }
                    }
                    pushParameterStack(v1);
                }
            } else {
                pushParameterStack( 0);
            }
            break;
            
        case SI_CC_entryToken:
            v1 = popParameterStack();
            v2 = popParameterStack();
            if( v1 != YRSHELL_DICTIONARY_INVALID && v2 < YRSHELL_DICTIONARY_LAST_INDEX && m_dictionaryList[ v2] != NULL) {
                v1 = m_dictionaryList[ v2]->getToken(v1);
                pushParameterStack( v1);
            } else {
                pushParameterStack(YRSHELL_DICTIONARY_INVALID);
            }
            break;
        case SI_CC_keyQ:
            if( m_Inq->valueAvailable()) {
                pushParameterStack( m_Inq->get());
                pushParameterStack( -1);
            } else {
                pushParameterStack( 0);
                pushParameterStack( 0);
            }
            break;
            
        case SI_CC_auxKeyQ:
            if( m_AuxInq->valueAvailable()) {
                pushParameterStack( m_AuxInq->get());
                pushParameterStack( -1);
            } else {
                pushParameterStack( 0);
                pushParameterStack( 0);
            }
            break;
        case SI_CC_auxIO:
            m_useAuxQueues = true;
            m_useTextOutput = false;
            break;
        case SI_CC_mainIO:
            m_useAuxQueues = false;
            m_useTextOutput = false;
            break;
        case SI_CC_emit:
            outChar( popParameterStack());
            break;
        case SI_CC_auxEmit:
            b = m_useAuxQueues;
            m_useAuxQueues = true;
            outChar( popParameterStack());
            m_useAuxQueues = b;
            break;
            
        case SI_CC_shellSize:
            pushParameterStack( shellSize());
            break;
        case SI_CC_printShellClass:
            m_outputStrPtr = shellClass();
            pushState( YRSHELL_OUTPUT_STR);
            break;

        case SI_CC_dictionarySize:
            pushParameterStack( m_DictionaryCurrent->getSize());
            break;
        case SI_CC_padSize:
            pushParameterStack( m_padSize);
            break;
        case SI_CC_numRegisters:
            pushParameterStack( m_numRegisters);
            break;
        case SI_CC_parameterStackSize:
            pushParameterStack( m_parameterStackSize);
            break;
        case SI_CC_returnStackSize:
            pushParameterStack( m_returnStackSize);
            break;
        case SI_CC_compileStackSize:
            pushParameterStack( m_compileStackSize);
            break;
        case SI_CC_inqSize:
            pushParameterStack( m_Inq->size());
            break;
        case SI_CC_auxInqSize:
            pushParameterStack( m_AuxInq->size());
            break;
        case SI_CC_outqSize:
            pushParameterStack( m_Outq->size());
            break;
        case SI_CC_auxOutqSize:
            pushParameterStack( m_AuxOutq->size());
            break;
 
        case SI_CC_dictionaryClear:
            m_DictionaryCurrent->reset();
            nextState( YRSHELL_BEGIN_IDLE);
            break;
 
        case SI_CC_setCommandEcho:
            m_commandEcho = popParameterStack() != 0;
            break;
        case SI_CC_setExpandCR:
            m_expandCR = popParameterStack() != 0;
            break;

        case SI_CC_systicks:
            pushParameterStack( HiResTimer::getSysticks());
            break;

        case SI_CC_micros:
            pushParameterStack( HiResTimer::getMicros());
            break;

        case SI_CC_millis:
            pushParameterStack( HiResTimer::getMillis());
            break;

        case SI_CC_bang:
        	v1 = popParameterStack();
        	v2 = popParameterStack();
        	if( v1 < m_numRegisters) {
        		m_Registers[ v1] = v2;
        	} else {
            	reset( __FILE__, __LINE__, "INVALID REGISTER");
	       	}
            break;
        case SI_CC_at:
        	v1 = popParameterStack();
        	if( v1 < m_numRegisters) {
        		pushParameterStack(m_Registers[ v1]);
        	} else {
            	reset( __FILE__, __LINE__, "INVALID REGISTER");
	       	}
            break;
         case SI_CC_clearStats:
        	v1 = popParameterStack();
        	S = Sliceable::getSlicePointer( v1);
        	if( S != NULL) {
        		S->resetTimer();
        		pushParameterStack( -1);
         	} else {
         		pushParameterStack( 0);
         	}
            break;
       case SI_CC_sliceStats:
        	v1 = popParameterStack();
        	S = Sliceable::getSlicePointer( v1);
        	if( S != NULL) {
        		pushParameterStack( S->getTimerAverage());
        		pushParameterStack( S->getTimerMax());
        		pushParameterStack( S->getTimerMin());
        		pushParameterStack( S->getTimerCount());
        		pushParameterStack( -1);
        	} else {
        		pushParameterStack( 0);
        	}
            break;
        case SI_CC_printSliceName:
        	v1 = popParameterStack();
        	S = Sliceable::getSlicePointer( v1);
        	if( S != NULL) {
                m_outputStrPtr = S->sliceName();
                pushState( YRSHELL_OUTPUT_STR);
         	}
            break;
            
        case SI_CC_find:
            P = getAddressFromToken(popParameterStack());
            pushParameterStack( find( P));
            break;
        case SI_CC_findEntry:
            P = getAddressFromToken(popParameterStack());
            pushParameterStack( findEntry( P));
            break;
        case SI_CC_fetchToken:
            v1 = popParameterStack( );
            v2 = fetchValueToken( v1);
            pushParameterStack( v2);
            break;

        case SI_CC_lshift:
            v1 = popParameterStack( );
            v2 = popParameterStack( );
            pushParameterStack( v2 << v1);
            break;
        case SI_CC_irshift:
            v1 = popParameterStack( );
            v2 = popParameterStack( );
            pushParameterStack( ((int32_t) v2) << ((int32_t) v1) );
            break;
        case SI_CC_rshift:
            v1 = popParameterStack( );
            v2 = popParameterStack( );
            pushParameterStack( v2 >> v1);
            break;
        case SI_CC_v_return:
            pushParameterStack( YRShellInterpreter::SI_CC_return);
            break;
        case SI_CC_v_uint16:
            pushParameterStack( YRShellInterpreter::SI_CC_uint16);
            break;
        case SI_CC_v_uint32:
            pushParameterStack( YRShellInterpreter::SI_CC_uint32);
            break;
        case SI_CC_v_nint16:
            pushParameterStack(YRShellInterpreter::SI_CC_nint16);
            break;
            
        case SI_CC_x_begin:
        case SI_CC_x_then:
        case SI_CC_noop:
            break;
            
        case SI_CC_v_if:
            pushParameterStack(YRShellInterpreter::SI_CC_x_if);
            break;
        case SI_CC_v_else:
            pushParameterStack(YRShellInterpreter::SI_CC_x_else);
            break;
        case SI_CC_v_then:
            pushParameterStack(YRShellInterpreter::SI_CC_x_then);
            break;
        case SI_CC_v_begin:
            pushParameterStack(YRShellInterpreter::SI_CC_x_begin);
            break;
        case SI_CC_v_until:
            pushParameterStack(YRShellInterpreter::SI_CC_x_until);
            break;
        case SI_CC_v_string:
            pushParameterStack(YRShellInterpreter::SI_CC_string);
            break;
        case SI_CC_isFunction:
            v1 = popParameterStack();
            v1 &= YRSHELL_DICTIONARY_MASK;
            pushParameterStack( v1 == YRSHELL_DICTIONARY_INTERPRETER_FUNCTION ||  v1 == YRSHELL_DICTIONARY_COMMON_FUNCTION || v1 == YRSHELL_DICTIONARY_EXTENSION_FUNCTION );
            break;
        case SI_CC_v_dictionaryMask:
            pushParameterStack(YRSHELL_DICTIONARY_MASK);
            break;
        case SI_CC_v_dictionaryRelative:
            pushParameterStack(YRSHELL_DICTIONARY_RELATIVE);
            break;

        case SI_CC_not:
            v1 = popParameterStack();
            v2 = ~ v1;
            pushParameterStack(v2);
            break;

        case SI_CC_dotRawStr:
            P = getAddressFromToken( popParameterStack());
            outRawString( P );
            break;

        case SI_CC_dotN:
            if( m_hexMode) {
                outUint32Xn( popParameterStack());
            } else {
                outInt32n( popParameterStack());
            }
            break;

        case SI_CC_setBaud:
            v1 = popParameterStack();
            v2 = popParameterStack();
            // Deprecated function
            break;
        case SI_CC_sysTicksPerSecond:
            v1 = HW_getSysTicksPerSecond();
            pushParameterStack( v1);
            break;

        case SI_CC_lastBufferSize:
#ifdef INPUT_BUFFER_EDITING
            pushParameterStack( m_lastBufferSize);
#else
            pushParameterStack( 0);
#endif
            break;
        case SI_CC_printMainFileName:
            m_outputStrPtr = mainFileName();
            pushState( YRSHELL_OUTPUT_STR);
            break;
        
        case SI_CC_setPromptEnable:
            m_promptEnable = popParameterStack( );
            break;
            
        case SI_CC_strBang:
            v1 = popParameterStack();
            v2 = popParameterStack();
            v1 =  v1 > ((uint32_t) (m_textBufferSize - 1)) ? m_textBufferSize - 1 : v1;
            v3 = m_textBufferSize - v1;
            if( m_textBufferSize > 0 && v3 > 1) {
                strncpy( &m_TextBuffer[ v1], getAddressFromToken(v2), v3);
                m_TextBuffer[ m_textBufferSize - 1] = '\0';
            }
            break;
        case SI_CC_charBang:
            v1 = popParameterStack();
            v2 = popParameterStack();
            if( v1 < m_textBufferSize) {
                m_TextBuffer[ v1 ] = v2;
                m_TextBuffer[ m_textBufferSize - 1] = '\0';
            }
            break;
        case SI_CC_charAt:
            v1 = popParameterStack();
            if( v1 < m_textBufferSize) {
                pushParameterStack( m_TextBuffer[ v1 ]);
            } else {
                pushParameterStack( m_TextBuffer[ v1 ]);
            }
            break;
        case SI_CC_textIO:
             m_useTextOutput = true;
            break;

#ifdef YRSHELL_INTERPRETER_FLOATING_POINT
        case SI_CC_dotf:
            outFloat(popFloat());
            break;
            
        case SI_CC_dote:
            outEngFloat(popFloat());
            break;
            
        case SI_CC_fLessThan:
            f1 = popFloat();
            f2 = popFloat();
            pushParameterStack(f2 < f1);
            break;
            
        case SI_CC_fLessEqual:
            f1 = popFloat();
            f2 = popFloat();
            pushParameterStack(f2 <= f1);
            break;
            
        case SI_CC_fEqual:
            f1 = popFloat();
            f2 = popFloat();
            pushParameterStack(f2 == f1);
            break;
            
        case SI_CC_fGreaterThan:
            f1 = popFloat();
            f2 = popFloat();
            pushParameterStack(f2 > f1);
            break;
            
        case SI_CC_fGreaterThanEqual:
            f1 = popFloat();
            f2 = popFloat();
            pushParameterStack(f2 >= f1);
            break;
            
        case SI_CC_fPlus:
            f1 = popFloat();
            f2 = popFloat();
            pushFloat(f2 + f1);
            break;
            
        case SI_CC_fMinus:
            f1 = popFloat();
            f2 = popFloat();
            pushFloat(f2 - f1);
            break;
            
        case SI_CC_fMultiply:
            f1 = popFloat();
            f2 = popFloat();
            pushFloat(f2 * f1);
            break;
            
        case SI_CC_fDivide:
            f1 = popFloat();
            f2 = popFloat();
            pushFloat(f2 / f1);
            break;
            
        case SI_CC_fSin:
            pushFloat( sinf(popFloat()));
            break;
            
        case SI_CC_fPI:
            pushFloat( 3.1415927f);
            break;
            
        case SI_CC_fCos:
            pushFloat( cosf(popFloat()));
            break;
            
        case SI_CC_fTan:
            pushFloat( tanf(popFloat()));
            break;
            
        case SI_CC_faSin:
            pushFloat( asinf(popFloat()));
            break;
            
        case SI_CC_faCos:
            pushFloat( acosf(popFloat()));
            break;
            
        case SI_CC_faTan:
            pushFloat( atanf(popFloat()));
            break;
            
        case SI_CC_fExp:
            pushFloat( expf(popFloat()));
            break;
            
        case SI_CC_fLog:
            pushFloat( logf(popFloat()));
            break;
            
        case SI_CC_fPow:
            f1 = popFloat();
            f2 = popFloat();
            pushFloat( powf(f2, f1) );
            break;
            
        case SI_CC_fLog10:
            pushFloat( log10f(popFloat()));
            break;
            
        case SI_CC_fRound:
            pushFloat( roundf(popFloat()));
            break;
            
        case SI_CC_floatToInt:
            v1 = ((int32_t) popFloat());
            pushParameterStack(v1);
            break;
            
        case SI_CC_intToFloat:
            v1 = popParameterStack();
            i = (int32_t) v1;
            pushFloat( i);
            break;
            
#endif
        default:
            reset( __FILE__, __LINE__);
            break;
    }
}
void YRShellInterpreter::CC_stringDef( ) {
    char c;
    uint16_t token = 0;
    bool tokenEmpty = true;
    m_DictionaryCurrent->addToken( SI_CC_string);
    while( 1) {
        c =  *m_saveptr++;
        if( c == '\\') {
            c = *m_saveptr++;
            switch( c) {
                case '\\':
                    c = '\\';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case 'x':
                    c = charToHex(*m_saveptr++) << 4;
                    c |= charToHex(*m_saveptr++);
                    break;
                default:
                    c = '0';
                    break;
            }
        } else if( c == '\'' || c == '\0' ) {
            if( tokenEmpty) {
                m_DictionaryCurrent->addToken( 0);
            } else {
                m_DictionaryCurrent->addToken( token);
            }
            break;
        }
        if( tokenEmpty) {
            token = (uint16_t) c;
            tokenEmpty = false;
        } else {
            token |= ((uint16_t) c) << 8;
            tokenEmpty = true;
            m_DictionaryCurrent->addToken( token);
        }
    }
}
void YRShellInterpreter::CC_if( ) {
    m_DictionaryCurrent->addToken( SI_CC_x_if);
    //m_DictionaryCurrent->addToken( SI_CC_jmpz);
    pushCompileStack(0x10000000 | YRSHELL_DICTIONARY_RELATIVE| m_DictionaryCurrent->getWordEnd());
    m_DictionaryCurrent->addToken( 0);
}
void YRShellInterpreter::CC_else( ) {
    uint32_t v = popCompileStack();
    if( (v & 0xF0000000) != 0x10000000) {
        shellERROR( __FILE__, __LINE__, "MISSING [");
    } else {
        m_DictionaryCurrent->addToken( SI_CC_x_else);
        //m_DictionaryCurrent->addToken( SI_CC_jmp);
        pushCompileStack(0x10000000 | YRSHELL_DICTIONARY_RELATIVE | m_DictionaryCurrent->getWordEnd());
        m_DictionaryCurrent->addToken( 0);
        m_DictionaryCurrent->setToken( v & YRSHELL_DICTIONARY_ADDRESS_MASK, YRSHELL_DICTIONARY_RELATIVE| m_DictionaryCurrent->getWordEnd());
    }
}
void YRShellInterpreter::CC_then( ) {
    uint32_t v = popCompileStack();
    if( (v & 0xF0000000) != 0x10000000) {
        shellERROR( __FILE__, __LINE__, "MISSING [");
    } else {
        m_DictionaryCurrent->addToken( SI_CC_x_then);
        m_DictionaryCurrent->setToken( v & YRSHELL_DICTIONARY_ADDRESS_MASK, YRSHELL_DICTIONARY_RELATIVE | m_DictionaryCurrent->getWordEnd());
    }
}
void YRShellInterpreter::CC_begin( ) {
    m_DictionaryCurrent->addToken( SI_CC_x_begin);
    pushCompileStack(0x20000000 | YRSHELL_DICTIONARY_RELATIVE | m_DictionaryCurrent->getWordEnd());
}
void YRShellInterpreter::CC_until( ) {
    uint32_t v = popCompileStack();
    if( (v & 0xF0000000) != 0x20000000) {
        shellERROR( __FILE__, __LINE__, "MISSING {");
    } else {
        m_DictionaryCurrent->addToken( SI_CC_x_until);
        m_DictionaryCurrent->addToken(YRSHELL_DICTIONARY_RELATIVE | (v & YRSHELL_DICTIONARY_ADDRESS_MASK));
    }
}
void YRShellInterpreter::interpretReset( ) {
    nextState( YRSHELL_INRESET);
    m_padCount = 0;
    m_token = NULL;
    m_saveptr = NULL;
    m_DictionaryCurrent->rollBack();
}
void YRShellInterpreter::reset( ) {
    if( m_state != YRSHELL_INRESET) {
        m_Inq->reset();
        m_AuxInq->reset();
        m_Outq->reset();
        m_AuxOutq->reset();
    
#ifdef YRSHELL_DEBUG
        //m_debugFlags = YRSHELL_DEBUG_STATE | YRSHELL_DEBUG_INPUT | YRSHELL_DEBUG_TOKEN;
        //m_debugFlags = YRSHELL_DEBUG_STATE | YRSHELL_DEBUG_TOKEN | YRSHELL_DEBUG_EXECUTE;
        m_debugFlags = 0;
#endif
        nextState( YRSHELL_INRESET);
        m_padCount = 0;
        m_token = NULL;
        m_saveptr = NULL;
    
        m_topOfStack = 0;
        m_returnTopOfStack = 0;
        m_compileTopOfStack = 0;
        m_stateTopOfStack = 0;
        m_hexMode = false;
        m_lastUseAuxQueues = false;
        m_useAuxQueues = false;
    
        m_PC = 0;
        //m_outputTimeoutInMilliseconds = 1000;
    
        m_outputStr0 = "\r" YRSHELL_VERSION "Compile Time: " __DATE__ " " __TIME__ " Main File: ";
        m_outputStr1 = mainFileName();
    }
}
void YRShellInterpreter::reset( const char* file, unsigned line) {
    reset();
    m_outputStr2 = getFileName( file);
    m_outputUint = line;
    m_outputUintValid = true;
}
void YRShellInterpreter::reset( const char* file, unsigned line, const char* message) {
    reset();
    m_outputStr2 = message;
    m_outputStr3 = getFileName( file);
    m_outputUint = line;
    m_outputUintValid = true;
}
void YRShellInterpreter::nextState(YRShellState n) {
#ifdef YRSHELL_DEBUG
    if( m_debugFlags & YRSHELL_DEBUG_STATE) {
        outString( "[[");
        outString(  stateDebugStrings[ m_state]);
        outString( "]->[");
        outString(stateDebugStrings[ n]);
        outString( "]]");
    }
#endif
    m_state = n;
    if( m_state <= YRSHELL_INVALID_STATE || m_state >= YRSHELL_LAST_INVALID_STATE) {
        shellERROR( __FILE__, __LINE__);
    }
}
void YRShellInterpreter::pushState( YRShellState n) {
    if( m_stateTopOfStack >= (sizeof(m_stateStack)/sizeof(m_stateStack[0])) ) {
        shellERROR( __FILE__, __LINE__);
    }
    m_stateStack[ m_stateTopOfStack++] = m_state;
    nextState( n);
}
void YRShellInterpreter::popState() {
    if( m_stateTopOfStack == 0 ) {
        shellERROR( __FILE__, __LINE__);
    }
    m_state = m_stateStack[ --m_stateTopOfStack];
    if( m_state <= YRSHELL_INVALID_STATE || m_state >= YRSHELL_LAST_INVALID_STATE) {
        shellERROR( __FILE__, __LINE__);
    }
}
void YRShellInterpreter::fillPadInternal( char c) {
    uint16_t i;
    if( c == '\r' || c == '\n') {
        for( i = 0; i < m_padCount; i++ ) {
            if( m_Pad[ i] != ' ' && m_Pad[ i] != '\t') {
                break;
            }
        }
        if( m_Pad[ i] == '/' && m_Pad[ i + 1] == '/') {
            m_padCount = 0;
            m_Pad[ 0] = '\0';
        }
#ifdef INPUT_BUFFER_EDITING
        if( m_lastBufferSize != 0) {
            m_lastBufferIndex = m_lastBufferFree;
            for( i = 0; i < m_padCount + 1; i++) {
                m_LastBuffer[ m_lastBufferFree++ ] = m_Pad[ i];
                if( m_lastBufferFree >= m_lastBufferSize) {
                    m_lastBufferFree = 0;
                }
            }
            j = m_lastBufferFree;
            for( i = 0; i < m_lastBufferSize + 1 && m_LastBuffer[ j] != '\0'; i++) {
                m_LastBuffer[  j++] = '\0';
                if( j >= m_lastBufferSize) {
                    j = 0;
                }
            }
        }
        nextState( YRSHELL_BEGIN_PARSING);
    } else if( c == '\x08' ) {
        if( m_padCount > 0) {
            m_Pad[ --m_padCount] = '\0';
            if( m_commandEcho) {
                outChar( ' ');
                outChar( '\x08');
            }
        }
    } else if( c == '\x05') {
        if( m_lastBufferSize != 0) {
            for( m_padCount = 0, i = 0, j = m_lastBufferIndex; i < (m_padSize - 1)  &&  m_LastBuffer[j] != '\0'; m_padCount++, i++) {
                m_Pad[ m_padCount] = m_LastBuffer[ j];
                j = (j < m_lastBufferSize - 1) ? j + 1 : 0;
            }
            m_Pad[ m_padCount] = '\0';
            m_lastBufferIndex = (m_lastBufferIndex == 0) ? m_lastBufferSize - 1 : m_lastBufferIndex - 1;
            for( i = 0; i < m_lastBufferSize &&  m_LastBuffer[ m_lastBufferIndex] == '\0'; i++ ) {
                m_lastBufferIndex = (m_lastBufferIndex == 0) ? m_lastBufferSize - 1 : m_lastBufferIndex - 1;
            }
            for( i = 0; i < m_lastBufferSize &&  m_LastBuffer[ m_lastBufferIndex] != '\0'; i++ ) {
                m_lastBufferIndex = (m_lastBufferIndex == 0) ? m_lastBufferSize - 1 : m_lastBufferIndex - 1;
            }
            m_lastBufferIndex = ((m_lastBufferIndex + 1) >= m_lastBufferSize) ? 0 : m_lastBufferIndex + 1;
            m_padCount = strlen( m_Pad);
            prompt();
            m_outputStrPtr = m_Pad;
            pushState( YRSHELL_OUTPUT_STR);
        }
    } else {
        m_Pad[ m_padCount++] = c;
        m_Pad[ m_padCount] = '\0';
    }
#else
    nextState( YRSHELL_BEGIN_PARSING);
    } else {
        m_Pad[ m_padCount++] = c;
        m_Pad[ m_padCount] = '\0';
    }
#endif
}
void YRShellInterpreter::fillPad( char c) {
#ifdef YRSHELL_DEBUG
    if( m_debugFlags & YRSHELL_DEBUG_INPUT) {
        outString("[YRShellInterpreter::fillPad ");
        outUint8X( c);
        outChar( ']');
    }
#endif
    if( m_commandEcho) {
        outChar( c);
    }
    if( m_padCount > (m_padSize - 4)) {
        reset( __FILE__, __LINE__, "INPUT BUFFER OVERFLOW");
    } else {
        fillPadInternal( c);
    }
}
#ifdef YRSHELL_DEBUG
void YRShellInterpreter::debugToken() {
    if( m_debugFlags &  YRSHELL_DEBUG_TOKEN) {
        outString("TOKEN: [");
        outString(m_token);
        outString( "]\r\n");
    }
}
#endif
void YRShellInterpreter::beginParsing(void) {
    m_saveptr = NULL;
    m_token = strtok_r(m_Pad, "\t ", &m_saveptr);
    if( m_token == NULL) {
        nextState( YRSHELL_BEGIN_IDLE  );
    } else {
        if( strcmp( m_token, ":")) {
            if( !m_DictionaryCurrent->newCompile("_")) {
                shellERROR( __FILE__, __LINE__);
            }
            if( m_compileTopOfStack) {
                shellERROR(__FILE__, __LINE__);
            }
            pushReturnStack( 0);
            m_PC = YRSHELL_DICTIONARY_CURRENT_RELATIVE | YRSHELL_DICTIONARY_CURRENT | m_DictionaryCurrent->getWordEnd();
#ifdef YRSHELL_DEBUG
            debugToken();
#endif
            if( processToken()) {
            	nextState( YRSHELL_PARSING);
            }
        } else {
            m_token = strtok_r( NULL, "\t ", &m_saveptr);
            if( m_token == NULL) {
                shellERROR( __FILE__, __LINE__, "NULL DEFINITION");
            } else {
                if( !m_DictionaryCurrent->newCompile(m_token)) {
                    shellERROR( __FILE__, __LINE__, "DICTIONARY FULL");
                } else {
#ifdef YRSHELL_DEBUG
                    if( m_debugFlags & YRSHELL_DEBUG_TOKEN) {
                        outString("\r\nCOMPILING ");
                    }
                    debugToken();
#endif
                    nextState( YRSHELL_COMPILING);
                }
            }
        }
    }
}
void YRShellInterpreter::setOutputTimeout( uint32_t t) {
    m_outputTimeoutInMilliseconds = t;
}

uint16_t YRShellInterpreter::outputSpace( ) {
    return  m_useTextOutput ? m_textBufferSize - strlen(m_TextBuffer): (m_useAuxQueues ? m_AuxOutq->free() : m_Outq->free());
}
bool YRShellInterpreter::isIdle( ) {
    return m_state == YRSHELL_IDLE;
}
 
void YRShellInterpreter::slice(void) {
    char c;
    if( m_lastUseAuxQueues != m_useAuxQueues) {
        m_lastUseAuxQueues = m_useAuxQueues;
    }
    if( ( (m_Outq->free() < (m_Outq->size()/2)) || (m_AuxOutq->free() < (m_AuxOutq->size()/2)) ) && m_state != YRSHELL_WAIT_FOR_OUTPUT_SPACE && m_state != YRSHELL_OUTPUT&& m_state != YRSHELL_OUTPUT_STR) {
        pushState( YRSHELL_WAIT_FOR_OUTPUT_SPACE);
        m_outputTimeout.setInterval(m_outputTimeoutInMilliseconds);
    }
    switch( m_state) {
    	case YRSHELL_NOT_INITIALIZED:
    		init();
    		break;
        case YRSHELL_BEGIN_IDLE:
            prompt( );
            CC_clearPad();
            m_padCount = 0;
            nextState( YRSHELL_IDLE);
            break;
        case YRSHELL_IDLE:
            if( m_requestUseMainQueues) {
                m_requestUseMainQueues = false;
                m_useAuxQueues = false;
            }
            if( m_requestUseAuxQueues) {
                m_requestUseAuxQueues = false;
                m_useAuxQueues = true;
            }
            if( m_useAuxQueues) {
                if( m_AuxInq->valueAvailable()) {
                    nextState( YRSHELL_FILLING_AUXPAD);
                }
            } else {
                if( m_Inq->valueAvailable()) {
                    nextState( YRSHELL_FILLING_PAD);
                }
            }
            break;
        case YRSHELL_FILLING_PAD:
            if( m_Inq->valueAvailable() ) {
                c = m_Inq->get();
                fillPad( c);
            }
            break;
        case YRSHELL_FILLING_AUXPAD:
            if( m_AuxInq->valueAvailable() ) {
                c = m_AuxInq->get();
                fillPad( c);
            }
            break;
        case YRSHELL_BEGIN_PARSING:
            beginParsing();
            break;
        case YRSHELL_PARSING:
            m_token = strtok_r(NULL, "\t ", &m_saveptr);
            processToken();
#ifdef YRSHELL_DEBUG
            debugToken();
#endif
            if( m_token == NULL) {
                nextState( YRSHELL_EXECUTING);
            }
            break;
        case YRSHELL_COMPILING:
            m_token = strtok_r(NULL, "\t ", &m_saveptr);
            processToken();
#ifdef YRSHELL_DEBUG
            debugToken();
#endif
            if( m_token == NULL) {
                if( m_compileTopOfStack) {
                    m_DictionaryCurrent->rollBack();
                    shellERROR(__FILE__, __LINE__, "INCOMPLETE CONTROL STRUCTURE");
                } else {
                    m_DictionaryCurrent->newCompileDone();
                    if( m_state != YRSHELL_OUTPUT_STR) {
                        nextState( YRSHELL_BEGIN_IDLE);
                    }
                }
            }
            break;
        case YRSHELL_EXECUTING:
            executing();
            break;
        case YRSHELL_WAIT_FOR_OUTPUT_SPACE:
            if( m_Outq->used() < (m_Outq->size()/2) && m_AuxOutq->used() < (m_AuxOutq->size()/2) ) {
                popState( );
                //nextState( m_lastState);
            } else if( m_outputTimeout.hasIntervalElapsed()) {
                if( m_outputTimeoutInMilliseconds != 0) {
                    shellERROR( __FILE__, __LINE__, "OUTPUT WAIT FOR SPACE TIMEOUT");
                } else {
                    m_Outq->drop(  m_Outq->size()/2 - m_Outq->free() );
                    m_AuxOutq->drop( m_Outq->size()/2 - m_AuxOutq->free());
                }
            }
            break;
        case YRSHELL_WAIT_DELAY:
            if( m_delayTimer.hasIntervalElapsed()) {
                popState( );
            }
            break;
        case YRSHELL_OUTPUT_STR:
            if( outputSpace() < 6) {
                if( m_outputTimeoutInMilliseconds == 0) {
                    m_Outq->drop(  m_Outq->size()/2 - m_Outq->free() );
                    m_AuxOutq->drop( m_Outq->size()/2 - m_AuxOutq->free());
                }
            } else {
                while( m_outputStrPtr != NULL && outputSpace() >= 4 ) {
                	if( *m_outputStrPtr) {
                		outChar( *m_outputStrPtr++);
                	}
                    if( *m_outputStrPtr == '\0') {
                        m_outputStrPtr = NULL;
                    }
                }
                if( m_outputStrPtr == NULL) {
                    popState( );
                }
            }
            break;

            
        case YRSHELL_OUTPUT:
            if(m_outputStr0 != NULL) {
                m_outputStrPtr = m_outputStr0;
                m_outputStr0 = NULL;
                pushState( YRSHELL_OUTPUT_STR);
            } else if(m_outputStr1 != NULL) {
                m_outputStrPtr = m_outputStr1;
                m_outputStr1 = NULL;
                pushState( YRSHELL_OUTPUT_STR);
           } else if( m_outputStr2 != NULL) {
                outChar( '\r');
                m_outputStrPtr = m_outputStr2;
                m_outputStr2 = NULL;
                pushState( YRSHELL_OUTPUT_STR);
            } else if( m_outputStr3 != NULL) {
                outChar( '\r');
                m_outputStrPtr = m_outputStr3;
                m_outputStr3 = NULL;
                pushState( YRSHELL_OUTPUT_STR);
            } else if( m_outputUintValid ) {
                m_outputUintValid = false;
                outInt32( m_outputUint);
            } else {
                outChar( '\r');
                popState( );
            }
            break;
        default:
            reset( __FILE__, __LINE__);
            break;
            
    }
    if( m_state == YRSHELL_INRESET) {
        nextState( YRSHELL_BEGIN_IDLE);
        pushState( YRSHELL_OUTPUT);
    }
}
bool YRShellInterpreter::processLiteralToken( ){
    bool rc = false;
    uint32_t value;
    if( stringToUnsigned( m_token, &value) || stringToUnsignedX( m_token, &value)
#ifdef YRSHELL_INTERPRETER_FLOATING_POINT
                || stringToFloat( m_token, &value)
#endif
            ) {
        rc = true;
        if( (value & 0xFFFF0000) == 0xFFFF0000) {
            if( !m_DictionaryCurrent->addToken( SI_CC_nint16)) {
                shellERROR( __FILE__, __LINE__);
            }
            if( !m_DictionaryCurrent->addToken( (uint16_t) value)) {
                shellERROR( __FILE__, __LINE__);
            }
        } else if( (value & 0xFFFF0000) == 0) {
            if( !m_DictionaryCurrent->addToken( SI_CC_uint16)) {
                shellERROR( __FILE__, __LINE__);
            }
            if( !m_DictionaryCurrent->addToken( (uint16_t) value)) {
                shellERROR( __FILE__, __LINE__);
            }
        } else {
            if( !m_DictionaryCurrent->addToken( SI_CC_uint32)) {
                shellERROR( __FILE__, __LINE__);
            }
            if( !m_DictionaryCurrent->addToken( (uint16_t) (0xFFFF & value))) {
                shellERROR( __FILE__, __LINE__);
            }
            if( !m_DictionaryCurrent->addToken( (uint16_t) (value >> 16))) {
                shellERROR( __FILE__, __LINE__);
            }
        }
    }
    return rc;
}
bool YRShellInterpreter::processToken( ){
	bool ret = true;
    if( m_token == NULL) {
        if( !m_DictionaryCurrent->addToken( SI_CC_return)) {
            shellERROR( __FILE__, __LINE__,  "DICTIONARY FULL");
        }
    } else {
        uint16_t rc = find( m_token);
        if( isCompileToken()) {
            executeToken(rc);
        } else {
            if( rc == YRSHELL_DICTIONARY_INVALID) {
                if( !processLiteralToken()) {
                    outString( "\rUNDEFINED: ");
                    m_outputStrPtr = m_token;
                    interpretReset();
                    nextState( YRSHELL_BEGIN_IDLE);
                    pushState( YRSHELL_OUTPUT_STR);
                    m_returnTopOfStack = 0;
                    m_compileTopOfStack = 0;
                    ret = false;
                }
            } else {
                if( (rc & YRSHELL_DICTIONARY_MASK) == YRSHELL_DICTIONARY_CURRENT) {
                    rc &= YRSHELL_DICTIONARY_ADDRESS_MASK;
                    rc |= YRSHELL_DICTIONARY_CURRENT_RELATIVE | YRSHELL_DICTIONARY_RELATIVE;
                }
                if( !m_DictionaryCurrent->addToken( rc)) {
                    shellERROR( __FILE__, __LINE__, "DICTIONARY FULL");
                }
            }
        }
    }
    return ret;
}
void YRShellInterpreter::executing( ) {
    if( (m_PC & YRSHELL_DICTIONARY_ADDRESS_MASK) == 0) {
        m_DictionaryCurrent->rollBack();
        nextState( YRSHELL_BEGIN_IDLE);
    } else {
#ifdef YRSHELL_DEBUG
        if( m_debugFlags & YRSHELL_DEBUG_EXECUTE) {
            outString("[");
            outUint16X(m_PC);
            outString( ":");
        }
#endif
        uint16_t token = fetchCurrentToken();
#ifdef YRSHELL_DEBUG
        if( m_debugFlags & YRSHELL_DEBUG_EXECUTE) {
            outUint16X(token);
            outString("]");
        }
#endif
        executeToken( token);
    }
}
const char* YRShellInterpreter::getAddressFromToken( uint16_t token) {
    const char *rc = NULL;
    uint16_t address = YRSHELL_DICTIONARY_ADDRESS_MASK & token;
    uint16_t mask = YRSHELL_DICTIONARY_MASK & token;
    uint16_t index = (mask >> YRSHELL_DICTIONARY_NUMBER_ADDRESS_BITS) & YRSHELL_DICTIONARY_LAST_INDEX;
    
    if( m_textBufferSize > 0 && mask == 0 && address < m_textBufferSize) {
        rc = &m_TextBuffer[ address];
    } else {
        if( index < YRSHELL_DICTIONARY_LAST_INDEX) {
            rc = m_dictionaryList[ index]->getAddress( address);
        } else {
            rc = m_dictionaryList[ (m_PC >> YRSHELL_DICTIONARY_NUMBER_TOKEN_BITS) & YRSHELL_DICTIONARY_LAST_INDEX ]->getAddress( address);
        }
    }
    return rc;
}
uint16_t YRShellInterpreter::getAbsoluteAddressToken( ) {
    return m_dictionaryList[ (m_PC >> YRSHELL_DICTIONARY_NUMBER_TOKEN_BITS) & YRSHELL_DICTIONARY_LAST_INDEX ]->getMask() | (m_PC & YRSHELL_DICTIONARY_ADDRESS_MASK);
}

uint16_t YRShellInterpreter::fetchValueToken( uint16_t tok) {
    uint16_t address = YRSHELL_DICTIONARY_ADDRESS_MASK & tok;
    uint16_t mask = YRSHELL_DICTIONARY_MASK & tok;
    uint16_t index = (mask >> YRSHELL_DICTIONARY_NUMBER_ADDRESS_BITS) & YRSHELL_DICTIONARY_LAST_INDEX;
    uint16_t token = YRSHELL_DICTIONARY_INVALID;

    if( index < YRSHELL_DICTIONARY_LAST_INDEX) {
        token = m_dictionaryList[ index]->getWord( address);
    } else {
        token = m_dictionaryList[ (m_PC >> YRSHELL_DICTIONARY_NUMBER_TOKEN_BITS) & YRSHELL_DICTIONARY_LAST_INDEX ]->getWord( address);
    }
    return token;
}
uint16_t YRShellInterpreter::fetchCurrentValueToken( ) {
    uint16_t token = fetchValueToken( m_PC);
    m_PC++;
    return token;
}

uint16_t YRShellInterpreter::fetchCurrentToken( ) {
    uint16_t token = fetchCurrentValueToken();
    if( token == YRSHELL_DICTIONARY_INVALID) {
        shellERROR( __FILE__, __LINE__);
    }
    return token;
}

void YRShellInterpreter::executeToken( uint16_t token ) {
    uint16_t mask = YRSHELL_DICTIONARY_MASK & token;
    switch( mask) {
        case YRSHELL_DICTIONARY_INTERPRETER_COMPILED:
            pushReturnStack( m_PC);
            m_PC = token | YRSHELL_DICTIONARY_INTERPRETER_COMPILED_RELATIVE;
            break;
        case YRSHELL_DICTIONARY_EXTENSION_COMPILED:
            pushReturnStack( m_PC);
            m_PC = token | YRSHELL_DICTIONARY_EXTENSION_COMPILED_RELATIVE;
            break;
        case YRSHELL_DICTIONARY_CURRENT:
            pushReturnStack( m_PC);
            m_PC = token | YRSHELL_DICTIONARY_CURRENT_RELATIVE;
            break;
        case YRSHELL_DICTIONARY_RELATIVE:
            switch( m_PC & YRSHELL_DICTIONARY_RELATIVE_MASK){
                case YRSHELL_DICTIONARY_CURRENT_RELATIVE:
                    pushReturnStack( m_PC);
                    m_PC = token | YRSHELL_DICTIONARY_CURRENT_RELATIVE;
                    break;
                case YRSHELL_DICTIONARY_INTERPRETER_COMPILED_RELATIVE:
                    pushReturnStack( m_PC);
                    m_PC = token | YRSHELL_DICTIONARY_INTERPRETER_COMPILED_RELATIVE;
                    break;
                case YRSHELL_DICTIONARY_EXTENSION_COMPILED_RELATIVE:
                    pushReturnStack( m_PC);
                    m_PC = token | YRSHELL_DICTIONARY_EXTENSION_COMPILED_RELATIVE;
                    break;
                    
            }
            break;
            
        case YRSHELL_DICTIONARY_INTERPRETER_FUNCTION:
            executeFunction(token);
            break;
        case YRSHELL_DICTIONARY_COMMON_FUNCTION:
            executeFunction(token);
            break;
        case YRSHELL_DICTIONARY_EXTENSION_FUNCTION:
            executeFunction(token);
            break;
        default:
            shellERROR( __FILE__, __LINE__);
            break;
    }
}
CircularQBase<char>& YRShellInterpreter::getInq() {
    return *m_Inq;
}
CircularQBase<char>& YRShellInterpreter::getAuxInq() {
    return *m_AuxInq;
}
CircularQBase<char>& YRShellInterpreter::getOutq() {
    return *m_Outq;
}
CircularQBase<char>& YRShellInterpreter::getAuxOutq() {
    return *m_AuxOutq;
}

void YRShellInterpreter::outCharRaw( const char c) {
    if( m_useTextOutput) {
        uint32_t s = (uint32_t) strlen( m_TextBuffer);
        if( s < ((uint32_t) (m_textBufferSize - 1)) ) {
            m_TextBuffer[ s++] = c;
            m_TextBuffer[ s] = '\0';
        } else {
            if(  m_state != YRSHELL_INRESET) {
                shellERROR( __FILE__, __LINE__, "TEXT OUTPUT BUFFER OVERFLOW");
            }
        }
    } else {
        if( m_useAuxQueues) {
            if( !m_AuxOutq->put( c) && m_state != YRSHELL_INRESET) {
                shellERROR( __FILE__, __LINE__, "AUX OUTPUT BUFFER OVERFLOW");
            }
        } else {
            if( !m_Outq->put( c) && m_state != YRSHELL_INRESET) {
                shellERROR( __FILE__, __LINE__, "OUTPUT BUFFER OVERFLOW");
            }
        }
    }
}
void YRShellInterpreter::outChar( const char c) {
    outCharRaw( c);
    if( m_expandCR && c == '\x0D') {
        outCharRaw( '\n');
    }
}
void YRShellInterpreter::outString( const char* P) {
    if( P != NULL) {
        while( *P != '\0') {
            outChar( *P++);
        }
    }
}
void YRShellInterpreter::outRawString( const char* P) {
    if( P != NULL) {
        while( *P != '\0') {
            if( *P == '\r') {
                outChar( '\\');
                outChar( 'r');
            } else if( *P == '\n') {
                outChar( '\\');
                outChar( 'n');
            } else if( *P == '\t') {
                outChar( '\\');
                outChar( 't');
            } else {
                outChar( *P);
            }
            P++;
        }
    }
}
#ifdef YRSHELL_INTERPRETER_FLOATING_POINT
float YRShellInterpreter::popFloat( ) {
    float rc;
    uint32_t* P = (uint32_t*) &rc;
    uint32_t v = popParameterStack();
    *P = v;
    return rc;
}
void  YRShellInterpreter::pushFloat( float f) {
    uint32_t* P = (uint32_t*) &f;
    pushParameterStack(*P);
}
bool YRShellInterpreter::stringToFloat( const char* P, uint32_t* V) {
    bool rc = false;
    float fv = 0.0f;
    int32_t bd = 0, nd = 0, ad = 0, exp = 0;
    const char *ep;
    if( *P != '\0') {
        P = stringToUnsignedInternal(P, (uint32_t*) &bd);
        if( *P++ == '.') {
            ep = stringToUnsignedInternal(P, (uint32_t*) &ad);
            nd = (uint32_t) (ep - P);
            if( *ep == '\0') {
                rc = true;
            } else if( *ep == 'e' || *ep == 'E') {
                ep++;
                ep = stringToUnsignedInternal( ep, (uint32_t*) &exp);
                if( *ep == '\0') {
                    rc = -1;
                }
            }
        }
    }
    if( rc) {
        fv = (powf(10.0f, nd) * ((float) bd) + ((float) ad)) * powf( 10.0f, exp - nd);
    }
    uint32_t* tp = ((uint32_t*) &fv);
    *V = *tp;
    return rc;
}
void YRShellInterpreter::floatToString( float num, char *s) {
    char *P = s;
    float d, fex;
    uint8_t i;
    int ex;
    
    if( num < 0) {
        *P++ = '-';
        num = 0.0f - num;
    } else {
        *P++ = ' ';
    }
    fex =log10f( num);
    ex = (int) fex;
    d = powf(10.f, (float) ex);
    num /= d;
    if( num < 1.0f) {
        num *= 10.0f;
        ex--;
    }
    if( num >= 1.0f) {
        i = (uint8_t) num;
        *P++ = 0x30 + i;
        num -= (float) i;
    } else {
        *P++ = '0';
    }
    *P++ = '.';
    unsignedToStringZero((uint32_t) (num * 1000000.0f), 6, P);
    P += 6;
    *P++ = 'e';
    if( ex < 0){
        *P++ = '-';
        ex = 0 - ex;
    } else {
        *P++ = '+';
    }
    unsignedToStringZero((uint32_t) ex, 2, P);
}
void YRShellInterpreter::floatEngToString( float num, char *s) {
    char sign, *P = s, *P1;
    float d, fex;
    int eex, ex, dc;
    
    if( num < 0) {
        sign = '-';
        num = 0.0f - num;
    } else {
        sign = ' ';
    }
    fex =log10f(num);
    eex = (int) fex;
    ex = 3* (eex/3);
    if( eex < 0){
        ex -= 3;
    }
    d = powf(10.f, (float) ex);
    dc = eex - ex;
    if(eex < 0 ) {
        dc--;
    }
    
    num /= d;
    unsignedToStringZero((uint32_t) num , 4, P);
    P1 = P;
    while( *P1 =='0') {
        *P1++ = ' ';
    }
    if( *P1 == '\0') {
        *--P1 = '0';
    }
    *--P1 = sign;
    P+= 4;
    *P++ = '.';
    unsignedToStringZero((uint32_t) (num * 1000000.0f), 6, P);
    P += 6;
    P1 = P;
    while( dc-- > 0) {
        *--P1 = '0';
    }
    *P++ = 'e';
    if( ex < 0) {
        *P++ = '-';
        ex = 0 - ex;
    } else {
        *P++ = '+';
    }
    unsignedToStringZero((uint32_t) ex, 2, P);
}
void YRShellInterpreter::outFloat( float num) {
    char buf[ 16];
    floatToString(num, buf);
    outString( buf);
}
void YRShellInterpreter::outEngFloat( float num) {
    char buf[ 16];
    floatEngToString(num, buf);
    outString( buf);
}
#endif
void YRShellInterpreter::outUint8( uint8_t v) {
    char buf[ 4];
    unsignedToString(v, 3, buf);
    outString( buf);
}
void YRShellInterpreter::outUint16( uint16_t v) {
    char buf[ 6];
    unsignedToString(v, 5, buf);
    outString( buf);
}
void YRShellInterpreter::outUint32( uint32_t v) {
    char buf[ 10];
    unsignedToString(v, 9, buf);
    outString( buf);
}
void YRShellInterpreter::outUint8X( uint8_t v) {
    char buf[ 5];
    unsignedToStringX(v, 2, buf);
    outString( buf);
}
void YRShellInterpreter::outUint16X( uint16_t v) {
    char buf[ 7];
    unsignedToStringX(v, 4, buf);
    outString( buf);
}
void YRShellInterpreter::outUint32X( uint32_t v) {
    char buf[ 11];
    unsignedToStringX(v, 8, buf);
    outString( buf);
    
}
void YRShellInterpreter::outUint32Xn( uint32_t v) {
    char *P, buf[ 11];
    unsignedToStringX(v, 8, buf);
    P = &buf[2];
    while( *P == '0') {
        P++;
    }
    if( *P == '\0') {
        P--;
    }
    *--P = 'x';
    *--P = '0';
    outString( P);
    
}
void YRShellInterpreter::outInt8( int8_t v) {
    char buf[ 5];
    intToString(v, 3, buf);
    outString( buf);
}
void YRShellInterpreter::outInt16( int16_t v) {
    char buf[ 7];
    intToString(v, 5, buf);
    outString( buf);
}
void YRShellInterpreter::outInt32( int32_t v) {
    char buf[ 12];
    intToString(v, 10, buf);
    outString( buf);
}
void YRShellInterpreter::outInt32n( int32_t v) {
    char *P, buf[ 12];
    intToString(v, 10, buf);
    P = buf;
    while( *P == ' ') {
        P++;
    }
    outString( P);
}
uint32_t YRShellInterpreter::popParameterStack( ) {
    uint32_t rc = 0;
    if(m_topOfStack > 0) {
        rc = m_ParameterStack[ --m_topOfStack];
    } else {
        shellERROR( __FILE__, __LINE__, "STACK UNDERFLOW\r\n");
    }
    return rc;
}
uint32_t YRShellInterpreter::popReturnStack( ) {
    uint32_t rc = 0;
    if(m_returnTopOfStack > 0) {
        rc = m_ReturnStack[ --m_returnTopOfStack];
    } else {
        shellERROR( __FILE__, __LINE__, "RETURN STACK UNDERFLOW\r\n");
    }
    return rc;
}
uint32_t YRShellInterpreter::popCompileStack( ) {
    uint32_t rc = 0;
    if(m_compileTopOfStack > 0) {
        rc = m_CompileStack[ --m_compileTopOfStack];
    } else {
        shellERROR( __FILE__, __LINE__, "COMPILE STACK UNDERFLOW\r\n");
    }
    return rc;
}
void YRShellInterpreter::pushParameterStack( uint32_t v) {
    if( m_topOfStack < m_parameterStackSize){
        m_ParameterStack[ m_topOfStack++] = v;
    } else {
        shellERROR( __FILE__, __LINE__, "STACK OVERFLOW\r\n");
    }
}
void YRShellInterpreter::pushReturnStack( uint32_t v) {
    if( m_returnTopOfStack < m_returnStackSize){
        m_ReturnStack[ m_returnTopOfStack++] = v;
    } else {
        shellERROR( __FILE__, __LINE__, "RETURN STACK OVERFLOW\r\n");
    }
}
void YRShellInterpreter::pushCompileStack( uint32_t v) {
    if( m_compileTopOfStack < m_compileStackSize){
        m_CompileStack[ m_compileTopOfStack++] = v;
    } else {
        shellERROR( __FILE__, __LINE__, "RETURN STACK OVERFLOW\r\n");
    }
}
void YRShellInterpreter::CC_clearPad() {
    memset(m_Pad, '\0',  m_padSize);
}
void YRShellInterpreter::setPrompt( const char* prompt ) {
    m_prompt = prompt;
}

