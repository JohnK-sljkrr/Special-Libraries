#include <Arduino.h>
//#include "/home/johnk/bin/arduino-1.6.5-r5/hardware/teensy/avr/cores/teensy3/Arduino.h"
//*************** Copyright (c) 2015 Universe Unlimited Microsystems***********
//
// File:         JKCmdParser.cpp
//
// Summary:
//
// Contents:
//
// Notes:
//
// Author:       John E. Kabat
//
// Last Edit:    12/18/2016 12:20:37	By: JohnK
//
// Revisions:
//
//*****************************************************************************
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "JKCmdParser.h"

char queue[BUFF_SIZE+2];                                // input buffer
int qidx = 0;                                           // next character index
const char *v_chars;                                    // valid characters
struct _TOKEN_LIST_S *p_vtlist;                         // valid tokens
int numTokens = 0;                                      //  number of tokes
int cSensitive = 0;                                     // case sensitive
int tcount = 0;                                         // number of tokens found in input
char tokens[(NUM_TOKENS+1)][(TOKEN_SIZE+1)];            // tokens found
char *token_q[NUM_TOKENS+1];

static long asc2val(char *p_str);
static bool is_number(char *p_str);
 bool is_bool(char *p_str);

static bool asc2bool(char *p_str);
JKCmdParser::JKCmdParser(const char *input_valid_chars, const struct _TOKEN_LIST_S *input_tokens, int num_tokens, int caseS)
{
    v_chars = input_valid_chars;
    p_vtlist = input_tokens;
    numTokens = num_tokens;
    cSensitive = caseS;
}

void JKCmdParser::reset(void)
{
    qidx = 0;
    tcount = 0;
    memset(queue, 0, BUFF_SIZE + 1);                    // clear string spaces
    memset(tokens, 0, TOKEN_SIZE * NUM_TOKENS);

}

void JKCmdParser::begin(void)
{
    reset();
}

bool JKCmdParser::enqueue(char c)
{
// Adds character c to input queue
// supports backspace for live input
    switch (c)
    {
        case 0x1b :
        {
            reset();
            Serial.println();
            break;
        }
        case 0 :                                        // end of input
        case 0x0a :
        case 0x0d :
        {
            queue[qidx] = 0;                            // teminate input
            tcount = count_tokens(queue);               // possible tokens in queue?
            if (tcount > 0)
            {
                extract_all();
                return (true);
            }
            else
                reset();
            break;
        }
        case 0x08:                                      // backspace?
        {
            if (qidx > 0)                               // if chars in queue
                queue[--qidx] = 0;                      // back up and erase last
            Serial.print(" \x08");
            break;
        }
        default:
        {
            queue[qidx++] = c;                          // add to buffer
            if (qidx == BUFF_SIZE)
                qidx -= 1;
            break;
        }
    }
    return (false);                                     // no tokens yet)
}



bool JKCmdParser::enqueueString(char *s)
{
    for (; *s != 0; s++)
    {
        if (enqueue(*s))
            return (true);
    }
    return (false);


}

char* JKCmdParser::queue_addr(void)
{
    return (queue);
// Returns hub address of queue for external .str() methods
}


int JKCmdParser::queue_len(void)
{
//Returns current length of input queue
    return (strlen(queue));
}

int JKCmdParser::count_tokens(char *p_str)
{
    int toks = 0;
    bool tflag = false;
    char c;

// Returns count of possible tokens in string
// -- does not compare to apps's token list

    for (; *p_str; p_str++)                             // iterate trhu string
    {

        c = *p_str;
        if (strchr(v_chars, c))                         // c found in valid chars
        {
            if (!tflag)                                 // and not already in token
            {
                tflag = true;                           // mark in token
                ++toks;                                 // new token, increment count
            }
        }
        else
            tflag = false;                              // mark not in token

    }
    if (!(toks < NUM_TOKENS))
    {
        return (0);
    }
    return (toks);
}
//
int JKCmdParser::token_count(void)
{
    return (tcount);
}
void JKCmdParser::extract_all(void)
{
// Extract all (up to MAX_TOKENS) tokens from input queue
//    char *p_tok;
    int tidx = 0;
//  bytefill(@tokens, 0, TOKEN_SIZE * NUM_TOKENS)           // clear tokens storage space
//
    tcount = count_tokens(queue);                       // count tokens
    if (tcount == 0)                                    // abort if no tokens in queue
        return;

    for (tidx = 0; tidx < (tcount); tidx++)
    {
        extract_token(queue, tidx);                     // extract token
    }
}

void JKCmdParser::extract_token(char *p_str, int tidx)
{
    int target;
    bool tflag = false;
    char c;
    char *p_tok;
    int  tlen = 1;
    bool quoted = false;
    char *pstrx;

// Extract token from buffer at p_str
// -- tidx is the target token (0..9)

    if (tidx >= NUM_TOKENS)                             // index is out-of-range
        return;
    else
        target = tidx;                                  // copy token index
    if (target > 0)
        target++;
    p_tok = token_addr(tidx);                           // point to token buffer
    memset(p_tok, 0, TOKEN_SIZE);                       // clear token buffer

    while (*p_str)                                      // iterate through buffer
    {
        // point to correct token in string
        pstrx = p_str;
        c = *p_str++;
        if (!quoted)
        {
            if (!strchr(v_chars, c))                    // c not valid?
                tflag = false;                          // mark not in a token
            else
            {
                // what we need to do here is for every start of a token
                // count down target and the loop until tflag is false
                // this will get us past the tokens we don't want

                if (!tflag)                             // if not in token and letter or number
                {
                    // see if correct token
                    tflag = true;                       // mark in a token
                    token_q[tidx] = pstrx;              // where token is in q
                    if (target > 0)                     // if not at target
                        --target;                       // decrement
                }
            }
        }
        // now target == 0
        // and tflag = true
        // get the token

        if ((target == 0 && tflag))
        {
            //           *p_tok++ = c;                                     // put current character into token buffer
            if (++tlen <= TOKEN_SIZE)                   // room in token?
            {
                *p_tok++ = c;                           // add to token buffer
            }
            else
                return;                                 // too big
        }
        else
        {
            // tflag is false
            if (target <= 0)                            // we are done with target
            {
                return;
            }
        }
    }
}

char* JKCmdParser::token_addr(int tidx)
{
// Returns start address of token string
// -- can return address of unused token

    if ((tidx >= 0) && (tidx < NUM_TOKENS))             // if in token storage space
        return (tokens[tidx]);                          // return hub adddress
    else
        return (NULL);
}

char* JKCmdParser::token_q_addr(int tidx)
{
// Returns start address of token in the input q
// -- can return address of unused token

    if ((tidx >= 0) && (tidx < NUM_TOKENS))             // if in token storage space
        return (token_q[tidx]);                         // return hub adddress
    else
        return (NULL);
}
//
int JKCmdParser::token_len(int tidx)
{
// Returns length of token at tidx

    return (strlen(token_addr(tidx)));
}


int JKCmdParser::get_token_id(char *p_str)
{
    int tidx = 0;

// Search known tokens list for string
// -- returns index (0..ntokens-1) if found
// -- returns -1 if no match

    for (; tidx < numTokens; tidx++)                    // iterate through known tokens
    {
        if (cSensitive)
        {
            // case sensitive
            if (strcmp(p_str, p_vtlist[tidx].tok_string) == 0) // if match found
                return (p_vtlist[tidx].tok);            // return token index
        }
        else
        {
            // case insensitive
            if (strcasecmp(p_str, p_vtlist[tidx].tok_string) == 0) // if match found
                return (p_vtlist[tidx].tok);            // return token index
        }
    }
    return (-1);
}

bool JKCmdParser::token_is_number(int tidx)
{
// Returns true if token at tidx is known number format
// -- binary (%) and hex ($) values must be indicated (e.g., %1010 or $0A)

    return (is_number(token_addr(tidx)));
}
bool JKCmdParser::token_is_bool(int tidx)
{
// Returns true if token at tidx is known  oolean valie
// 0/1, yes/no, on/off. true/false

    return (is_bool(token_addr(tidx)));
}
long int  JKCmdParser::token_value(int tidx)
{
// Returns value of token at tidx
// -- 0 if not a number
// -- use token_is_number() before if 0 is valid

    if (token_is_number(tidx))
        return (asc2val(token_addr(tidx)));

    return (0);
}

double  JKCmdParser::token_value_float(int tidx)
{
// Returns value of token at tidx
// -- 0 if not a number
// -- use token_is_number() before if 0 is valid

    if (token_is_number(tidx))
        return (strtod(token_addr(tidx), NULL));

    return (0);
}

bool  JKCmdParser::token_value_bool(int tidx)
{
// Returns value of token at tidx
// -- 0 if not a number
// -- use token_is_number() before if 0 is valid
    if (token_is_bool(tidx))
        return (asc2bool(token_addr(tidx)));

    return (false);
}
//long int value(char *p_str)
//{
//// Convert string to value
//// -- must be correctly formatted
////
//    return (asc2val(p_str));
//}
bool is_decimal(char *p_str)
{
    char c;
// Returns true if string is decimal format


    for (; *p_str; p_str++)
    {
        c = *p_str;
        if (!isdigit(c))
            return (false);
    }
    return (true);
}

bool is_float(char *p_str)
{
    char c;
    bool haveDP = false;
// Returns true if string is float format ([+-]x.y)


    for (; *p_str; p_str++)
    {
        c = *p_str;
        if (!isdigit(*p_str))
        {
            if (c != '.')
            {
                return (false);
            }
            if (haveDP)
            {
                return (false);
            }
            haveDP = true;
        }
    }
    return (haveDP);
}

typedef struct _BoolVal
{
    char *name;
    bool b;
} BoolVal;
BoolVal boolValues[] =
{
    { (char *)"0", false },
    { (char *)"off", false },
    { (char *)"false", false },
    { (char *)"no", false },
    { (char *)"1", true },
    { (char *)"on", true },
    { (char *)"true", true },
    { (char *)"yes", true },
};
static BoolVal *boolValueFound;
const int BV_SIZE = (sizeof(boolValues) / sizeof(BoolVal));
bool is_bool(char *p_str)
{
    char c[3];
    char s[17];

    strcpy(c," ");
    strcpy(s, "");
    boolValueFound = &boolValues[0];

    for (; *p_str; p_str++)
    {
        c[0] = *p_str;
        strncat(s, c, sizeof(s));
    }
    for (int i = 0; i < BV_SIZE;i++)
    {
        if (strcmp(boolValues[i].name,s)==0)
        {
            boolValueFound = &boolValues[i];
            return (true);
        }
    }
    return (false);
}

bool is_binary(char *p_str)
{
    char c;
// Returns true if string is binaryformat


    for (; *p_str; p_str++)
    {
        c = *p_str;
        if (!((c >= '0') && (c <= '1')))
            return (false);
    }
    return (true);
}


static bool is_hex(char *p_str)
{
    char c;
// Returns true if string is decimal format


    for (; *p_str; p_str++)
    {
        c = *p_str;
        if (!isxdigit(c))
            return (false);
    }
    return (true);
}
static bool is_number(char *p_str)
{
    char c;
// Returns true if string is number in known format (dec, bin, or hex)
    c = *p_str++;
    if (c == '$')                                       // hex indicator?
        return (is_hex(p_str));

    if (c == '%')                                       // binary indicator?
        return (is_binary(p_str));

    if ((c == '-') || (c == '+'))                       // +- sign?
    {
        if (is_decimal(p_str))
        {
            return (true);
        }
        if (is_float(p_str))
        {
            return (true);
        }
    }

    if (c == '0')                                       // 0 leadin?
    {
        if (tolower(*(p_str)) == 'x')                   // 0x for hex?
        {
            return (is_hex(p_str + 1));
        }
        if (tolower(*(p_str)) == 'b')                   // 0b for binary
        {
            return (is_binary(p_str + 1));
        }
    }
    if (isdigit(c))                                     // digit 0..9
    {
        if (is_decimal(p_str))
        {
            return (true);
        }
        if (is_float(p_str))
        {
            return (true);
        }
    }
    return (false);
}

//***************************
//  { --------------------- }
//  {  Numeric conversions  }
//  { --------------------- }
//
//***************************
static bool asc2bool(char *p_str)
{
    is_bool(p_str);
    return (boolValueFound->b);
}
static long asc2val(char *p_str)
{
    char c;

// Returns value of numeric string
// -- p_str is pointer to string
// -- binary (%) and hex ($) must be indicated

    while (*p_str)
    {
        c = *p_str;
        switch (c)
        {
            case ' ':                                   // skip leading space(s)
            {
                p_str++;
                break;
            }

            case '0' :                                  // leading for 0b and 0x
            {
                if (tolower(*(p_str + 1)) == 'x')
                {
                    // hex
                    return (strtol(p_str + 2, NULL, 16));
                }
                if (tolower(*(p_str + 1)) == 'b')
                {
                    // binary
                    return (strtol(p_str + 2, NULL, 2));
                }
                return (strtol(p_str, NULL, 10));
            }
            case '+':
            case '-':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':                                   // found decimal value
            {
                return (strtol(p_str, NULL, 10));
            }
            case  '%':                                  // found binary value
            {
                return (strtol(p_str + 1, NULL, 2));
            }
            case '$':                                   // found hex value
            {
                return (strtol(p_str + 1, NULL, 16));
            }

            default :                                   // abort on bad character
                return (0);
        }
    }
    return (0);
}


