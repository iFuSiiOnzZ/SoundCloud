#include "js_reader.h"

#define cast(type, data) (type)(data)
#define DefaultName "root"

typedef enum JS_TOKENS
{
    Token_Unknown,          //
    Token_Field,            // "

    Token_Comma,            // ,
    Token_Colon,            // :

    Token_OpenBraket,       // [
    Token_CloseBraket,      // ]

    Token_OpenBrace,        // {
    Token_CloseBrace,       // }

    Token_EndOfStream       // '\0'
} JS_TOKENS;

typedef struct JS_TOKEN
{
    char *Name;             // NOTE(Andrei): Token Name
    size_t Size;            // NOTE(Andrei): Size of the name, we don't use null terminated string
    JS_TOKENS Type;         // NOTE(Andrei): Token types, '[' | ']', '{' | '}', ':' , '"', ',', '\0'
} JS_TOKEN;

typedef struct queue_t      // NOTE(Andrei): This is used to store the current json node as a parent.
{                           // if we found and array or an object we push the actual json node to queue,
    struct queue_t *Prev;   // this way when we end parsing the structure we came back to node where we
    JS_NODE *Data;          // were before parsing the structure
    int inArray;
} queue_t;

inline bool IsEndOfLine(char C)
{
    return (C == '\n') || (C == '\r');
}

inline bool IsWhiteSpace(char C)
{
    return ((C == ' ') || (C == '\t') || (C == '\v') || (C == '\f') || IsEndOfLine(C));
}

inline bool IsAlpha(char C)
{
    return (((C >= 'a') && (C <= 'z')) || ((C >= 'A') && (C <= 'Z')));
}

inline bool IsNumber(char C)
{
    return ((C >= '0') && (C <= '9'));
}

inline void RemoveSpace(JS_TOKENIZER *pTokenizer)
{
    while (IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }
}

static JS_TOKEN GetToken(JS_TOKENIZER *pTokenizer)
{
    JS_TOKEN Token = { 0 };
    RemoveSpace(pTokenizer);

    Token.Name = pTokenizer->At;
    Token.Size = 1;

    char ActualChar = pTokenizer->At[0];
    ++pTokenizer->At;

    switch(ActualChar)
    {
        case '\0' : Token.Type = Token_EndOfStream; break;

        case '{'  : Token.Type = Token_OpenBrace; break;
        case '}'  : Token.Type = Token_CloseBrace; break;

        case '['  : Token.Type = Token_OpenBraket; break;
        case ']'  : Token.Type = Token_CloseBraket; break;

        case '"'  : Token.Type = Token_Field; break;

        case ','  : Token.Type = Token_Comma; break;
        case ':'  : Token.Type = Token_Colon; break;
        default   : Token.Type = Token_Unknown; break;
    }

    return Token;
}

static void ParseFieldName(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    RemoveSpace(pTokenizer);
    pNode->Name = pTokenizer->At;

    while(pTokenizer->At[0] != '\0' && pTokenizer->At[0] != '"')
    {
        ++pTokenizer->At;
    }

    json_set_name_size(pNode->Size, cast(int, pTokenizer->At - pNode->Name));
    if(pTokenizer->At[0] == '"') ++pTokenizer->At;
}

static void ParseOtherData(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = (pTokenizer->At - 1);
    pNode->Type = JS_UNDEFINED;

    while(pTokenizer->At[0] && pTokenizer->At[0] != ',' && pTokenizer->At[0] != ']' && pTokenizer->At[0] != '}' && !IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
}

static void ParseString(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = pTokenizer->At;
    pNode->Type = JS_STRING;

    while(pTokenizer->At[0] != '\0' && pTokenizer->At[0] != '"')
    {
        if(pTokenizer->At[0] == '\\') ++pTokenizer->At;
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
    if(pTokenizer->At[0] == '"') ++pTokenizer->At;
}

static void GetFieldData(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    RemoveSpace(pTokenizer);
    char ActualChar = *pTokenizer->At++;

    switch(ActualChar)
    {
        case '"'  :
        {
            ParseString(pTokenizer, pNode);
        } break;

        case 'n'  :
        case 'N'  :
        {
            ParseOtherData(pTokenizer, pNode);
            pNode->Type = JS_NULL;
        } break;

        case 't'  :
        case 'T'  :
        case 'f'  :
        case 'F'  :
        {
            ParseOtherData(pTokenizer, pNode);
            pNode->Type = JS_BOOL;
        }break;

        case '-'  :
        case '0'  :
        case '1'  :
        case '2'  :
        case '3'  :
        case '4'  :
        case '5'  :
        case '6'  :
        case '7'  :
        case '8'  :
        case '9'  :
        {
            ParseOtherData(pTokenizer, pNode);
            pNode->Type = JS_NUMBER;
        } break;

        case '{'  :
        {
            pNode->Type = JS_OBJECT;
            --pTokenizer->At;
        } break;

        case '['  :
        {
            pNode->Type = JS_ARRAY;
            --pTokenizer->At;
        } break;

        case '\0' :
        default   :  break;
    }
}

static void push(queue_t **pQueue, JS_NODE *pNode, int inArray)
{
    if(*pQueue == NULL)
    {
        *pQueue = (queue_t *) malloc (sizeof(queue_t));
        (*pQueue)->Prev = NULL;
    }
    else
    {
        queue_t *pNewNode = (queue_t *) malloc (sizeof(queue_t));
        pNewNode->Prev = *pQueue;
        *pQueue = pNewNode;
    }

    (*pQueue)->inArray = inArray;
    (*pQueue)->Data = pNode;
}

static JS_NODE * pop(queue_t **pQueue, int *inArray)
{
    if(*pQueue == NULL)
    {
        return NULL;
    }

    JS_NODE *pDataNode = (*pQueue)->Data;
    queue_t *pQueueNode = *pQueue;

    *inArray = pQueueNode->inArray;
    *pQueue = (*pQueue)->Prev;

    free(pQueueNode);
    return pDataNode;
}

void json_parser(JS_NODE *pRootNode, JS_TOKENIZER *pTokenizer)
{
    int inArray = 0;
    queue_t *pQueue = NULL;

    while(true)
    {
        RemoveSpace(pTokenizer);
        JS_TOKEN Token = GetToken(pTokenizer);

        switch(Token.Type)
        {
            case Token_EndOfStream:
            {
                return;
            } break;

            case Token_CloseBraket: case Token_CloseBrace:
            {
                pRootNode = pop(&pQueue, &inArray);
            } break;

            case Token_OpenBrace: case Token_OpenBraket:
            {
                if(pRootNode->Type == JS_UNDEFINED)
                {
                    pRootNode->Name = DefaultName;
                }

                pRootNode->Type = Token.Type == Token_OpenBraket ? JS_ARRAY : JS_OBJECT;
                pRootNode->Childs = (JS_NODE *) malloc (sizeof(JS_NODE));

                push(&pQueue, pRootNode, inArray);
                inArray = pRootNode->Type == JS_ARRAY;

                json_default(pRootNode->Childs);
                pRootNode = pRootNode->Childs;
            } break;

            case Token_Comma:
            {
                pRootNode->Sibling = (JS_NODE *) malloc (sizeof(JS_NODE));
                json_default(pRootNode->Sibling);
                pRootNode = pRootNode->Sibling;
            } break;

            case Token_Field:
            {
                if(inArray) --pTokenizer->At, GetFieldData(pTokenizer, pRootNode);
                else ParseFieldName(pTokenizer, pRootNode);
            } break;

            case Token_Colon:
            {
                GetFieldData(pTokenizer, pRootNode);
            } break;

            default:
            {
                --pTokenizer->At;
                GetFieldData(pTokenizer, pRootNode);
            } break;
        }
    }
}

void json_clear(JS_NODE *pNode)
{
    if(pNode == NULL) return;

    json_clear(pNode->Childs);
    json_clear(pNode->Sibling);

    free(pNode);
}

static void json_sanitize_end_of_string(JS_NODE * pNode)
{
    if (pNode == NULL) return;

    json_sanitize_end_of_string(pNode->Childs);
    json_sanitize_end_of_string(pNode->Sibling);

    int n = json_get_name_size(pNode->Size);
    int v = json_get_data_size(pNode->Size);

    if (n) pNode->Name[n] = 0;
    if (v) pNode->Value[v] = 0;
}

static void json_sanitize_special_chars(JS_NODE * pNode)
{
    if (pNode == NULL) return;

    json_sanitize_special_chars(pNode->Childs);
    json_sanitize_special_chars(pNode->Sibling);

    char *r = pNode->Value;
    char *w = r;

    while (r && *r)
    {
        if (*r == '\\') ++r; // NOTE(Andrei): Should we also check the next character?
        *w++ = *r++;
    }

    if(w) *w = 0;
}

void json_sanitize(JS_NODE * pNode)
{
    json_sanitize_end_of_string(pNode);
    json_sanitize_special_chars(pNode);
}

void json_default(JS_NODE *pNode)
{
    pNode->Childs = NULL;
    pNode->Sibling = NULL;

    pNode->Name = NULL;
    pNode->Value = NULL;

    pNode->Size = 0;
    pNode->Type = JS_UNDEFINED;
}

JS_NODE * json_root()
{
    JS_NODE *pRootNode = (JS_NODE *) malloc (sizeof(JS_NODE));
    json_default(pRootNode);

    return pRootNode;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2)) s1++, s2++;
    return *(const unsigned char *) s1 - *(const unsigned char *) s2;
}

JS_NODE * json_find_sibling(JS_NODE *pNode, char *pQuery)
{
    while(pNode && strcmp(pQuery, pNode->Name))
    {
        pNode = pNode->Sibling;
    }

    return pNode;
}

JS_NODE * json_child_n(JS_NODE *pNode, int i)
{
    while(pNode && i--)
    {
        pNode = pNode->Sibling;
    }

    if(pNode != NULL && (pNode->Type == JS_OBJECT) || pNode->Type == JS_ARRAY) return pNode->Childs;
    return pNode;
}

int json_count_siblings(JS_NODE *pNode)
{
    int i = 0;

    while(pNode)
    {
        pNode = pNode->Sibling;
        i++;
    }

    return i;
}

char * json_value(JS_NODE *pNode, char *pQuery, int *pArray /* = 0 */, int ArrayCount /* = 0 */)
{
    char pBlock[32] = { 0 };
    char *p = pQuery, i = 0;

    while(p && *p && *p != '.')
    {
        pBlock[i++] = *p++;
    }

    JS_NODE *pSibling = json_find_sibling(pNode, pBlock);
    if(pSibling == NULL) return NULL;

    if(pSibling->Type == JS_OBJECT && *p)
    {
        return json_value(pSibling->Childs, p + 1, pArray, ArrayCount);
    }
    else if(pSibling->Type == JS_ARRAY && *p)
    {
        return json_value(json_child_n(pSibling->Childs, pArray[0]), p + 1, pArray + 1, ArrayCount - 1);
    }

    return pSibling->Type == JS_ARRAY ? json_child_n(pSibling->Childs, pArray[0])->Value : pSibling->Value;
}

int json_size(JS_NODE *pNode, char *pQuery, int *pArray /* = 0 */, int ArrayCount /* = 0 */)
{
    char pBlock[32] = { 0 };
    char *p = pQuery, i = 0;

    while(p && *p && *p != '.')
    {
        pBlock[i++] = *p++;
    }

    JS_NODE *pSibling = json_find_sibling(pNode, pBlock);
    if(pSibling == NULL) return 0;

    if(pSibling->Type == JS_OBJECT && *p)
    {
        return json_size(pSibling->Childs, p + 1, pArray, ArrayCount);
    }
    else if(pSibling->Type == JS_ARRAY && *p)
    {
        return json_size(json_child_n(pSibling->Childs, pArray[0]), p + 1, pArray + 1, ArrayCount - 1);
    }

    return json_count_siblings((pSibling->Type == JS_ARRAY) ? pSibling->Childs : pSibling);
}

static void json_print_internal(JS_NODE *pNode, char *pNodeName, JS_TYPES jNodeType, int NodeHasSiblings, int Level)
{
    if(jNodeType == JS_ARRAY && pNodeName)
    {
        if(strcmp(pNodeName, "root")) printf("%*s\"%s\" :\n", 4 * Level, " ", pNodeName);
        printf("%*s%s\n", 4 * Level, " ", "[");
    }
    else if(jNodeType == JS_OBJECT && pNodeName)
    {
        if(strcmp(pNodeName, "root"))printf("%*s\"%s\" :\n", 4 * Level, " ", pNodeName);
        printf("%*s%s\n", 4 * Level, " ", "{");
    }

    while(pNode)
    {
        JS_NODE *isNoLastSibling = pNode->Sibling;

        if(pNode->Type == JS_OBJECT || pNode->Type == JS_ARRAY) 
        {
            json_print_internal(pNode->Childs, pNode->Name, pNode->Type, pNode->Sibling != NULL, Level + 1);
             pNode = pNode->Sibling; continue;
        }

        printf("%*s", 4 + 4 * Level, " ");
        if(pNode->Name) printf("\"%s\" : ", pNode->Name);

        if(pNode->Type == JS_STRING) printf("\"%s\"", pNode->Value);
        else printf("%s", pNode->Value);

        printf("%s\n", isNoLastSibling ? "," : "");
        pNode = pNode->Sibling;
    }

    if(jNodeType == JS_ARRAY && pNodeName)
    {
        printf("%*s%s", 4 * Level, " ", "]");
        printf("%s\n", NodeHasSiblings ? "," : "");
    }
    else if(jNodeType == JS_OBJECT && pNodeName)
    {
        printf("%*s%s", 4 * Level, " ", "}");
        printf("%s\n", NodeHasSiblings ? "," : "");
    }
}

void json_print(JS_NODE *pNode)
{
    json_print_internal(pNode, NULL, JS_UNDEFINED, pNode->Sibling != NULL, -1);
}