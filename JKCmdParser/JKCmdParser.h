//*************** Copyright (c) 2015 Universe Unlimited Microsystems***********
//
// File:         JKCmdParser.h
//
// Summary:
//
// Contents:
//
// Notes:
//
// Author:       John E. Kabat
//
// Last Edit:    12/9/2016 12:32:19	By: JohnK
//
// Revisions:
//
//*****************************************************************************

#ifndef JKCmdParser_h
#define JKCmdParser_h

#if defined(__MK20DX256__)
//#pragma message ("Teensy 3x")
#define BUFF_SIZE       512
#define TOKEN_SIZE      80
#define NUM_TOKENS      32
#else
#define BUFF_SIZE       80
#define TOKEN_SIZE      12
#define NUM_TOKENS      16
#endif
struct _TOKEN_LIST_S
{
    const int tok;
    const char *tok_string;
};


#define TOKEN_LIST(name) const struct _TOKEN_LIST_S name[] = {

#define TOKEN_ENTRY(a,b)   {a, (const char * )  b , },
#define TOKEN_LIST_END };

class JKCmdParser
{
public:
    JKCmdParser(const char *input_valid_chars, const struct _TOKEN_LIST_S *tlist, int num_tokens, int caseS);
    void begin(void);
    void reset(void);
    bool enqueue(char c);
    bool enqueueString(char *s);
    bool token_is_number(int tidx);
    bool token_is_bool(int tidx);
    long int token_value(int tidx);
    double token_value_float(int tidx);
    bool token_value_bool(int tidx);
    int token_count(void);
    char* token_addr(int tidx);
    int token_len(int tidx);
    int get_token_id(char *p_str);
    char* token_q_addr(int tidx);
private:
    char queue[BUFF_SIZE+1];
    char tokens[(NUM_TOKENS+1)][(TOKEN_SIZE+1)];        // tokens found
    int maxInLen;
    int inLen;
    char *v_Chars;
    const struct _TOKEN_LIST_S *p_vtlist;               // ptr to token list
    int numTokens;
    int cSensitive;

    char* queue_addr(void);
    int queue_len(void);
    int count_tokens(char *p_str);
    void extract_all(void);
    void extract_token(char *p_str, int tidx);

};

#endif


