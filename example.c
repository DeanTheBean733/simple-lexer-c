#include <string.h>

#include "simple-lexer.h"

typedef enum {
    PUNCT_PLUS,
    PUNCT_MULT,
    PUNCT_EQUALS,
    PUNCT_OPAREN,
    PUNCT_CPAREN,
    PUNCT_OCURLY,
    PUNCT_CCURLY,
    PUNCT_SEMICOLON,
    COUNT_PUNCTS,
} Punct_Index;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(COUNT_PUNCTS == 8, "Punctuation count changed");
#else
typedef char static_assertion_COUNT_PUNCTS[(COUNT_PUNCTS == 8) ? 1 : -1];
#endif

const char *puncts[COUNT_PUNCTS] = {
    [PUNCT_PLUS]      = "+",
    [PUNCT_MULT]      = "*",
    [PUNCT_OPAREN]    = "(",
    [PUNCT_CPAREN]    = ")",
    [PUNCT_OCURLY]    = "{",
    [PUNCT_CCURLY]    = "}",
    [PUNCT_EQUALS]    = "==",
    [PUNCT_SEMICOLON] = ";",
};

typedef enum {
    KEYWORD_IF,
    KEYWORD_RETURN,
    COUNT_KEYWORDS,
} Keyword_Index;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(COUNT_KEYWORDS == 2, "Keyword count changed");
#else
typedef char static_assertion_COUNT_KEYWORDS[(COUNT_KEYWORDS == 2) ? 1 : -1];
#endif

const char *keywords[COUNT_KEYWORDS] = {
    [KEYWORD_IF]     = "if",
    [KEYWORD_RETURN] = "return",
};

const char *sl_comments[] = {
    "//",
    "#",
};

SimpleLexer_MLComments ml_comments[] = {
    {"/*", "*/"},
};

int main(void) {
    const char *file_path = "example.input";
    const char *content =
        "#include <stdio.h>\n"
        "if (a == 180*2 + 50) { // single line comment\n"
        "    /* multi\n"
        "     * line\n"
        "     * comment\n"
        "     */\n"
        "    return b;\n"
        "}\n";

    SimpleLexer lexer = simplelexer_create(file_path, content, strlen(content));
    lexer.puncts = puncts;
    lexer.puncts_count = SIMPLELEXER_ARRAY_LEN(puncts);
    lexer.keywords = keywords;
    lexer.keywords_count = SIMPLELEXER_ARRAY_LEN(keywords);
    lexer.sl_comments = sl_comments;
    lexer.sl_comments_count = SIMPLELEXER_ARRAY_LEN(sl_comments);
    lexer.ml_comments = ml_comments;
    lexer.ml_comments_count = SIMPLELEXER_ARRAY_LEN(ml_comments);

    SimpleLexer_Token token = {0};
    while (simplelexer_get_token(&lexer, &token)) {
        lexer.diagf(token.loc, "INFO", "%s: %.*s",
                    simplelexer_kind_name(SIMPLELEXER_KIND(token.id)),
                    (int)(token.end - token.begin), token.begin);
    }

    if (!simplelexer_expect_id(&lexer, token, SIMPLELEXER_END)) {
        return 1;
    }
    
    return 0;
}
