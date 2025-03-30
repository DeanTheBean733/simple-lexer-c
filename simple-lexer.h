 #ifndef SIMPLE_LEXER_H_
 #define SIMPLE_LEXER_H_
 
 #include <assert.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include <stdio.h>
 #include <string.h>
 #include <ctype.h>
 #include <stdarg.h>
 
 #define SIMPLELEXER_ARRAY_LEN(xs) (sizeof(xs) / sizeof((xs)[0]))
 #define simplelexer_return_defer(value) do { result = (value); goto defer; } while(0)
 
 typedef struct {
     char *items;
     size_t count;
     size_t capacity;
 } SimpleLexer_StringBuilder;

 #ifndef SIMPLELEXER_ASSERT
 #define SIMPLELEXER_ASSERT assert
 #endif
 
 #ifndef SIMPLELEXER_REALLOC
 #define SIMPLELEXER_REALLOC realloc
 #endif
 
 #ifndef SIMPLELEXER_FREE
 #define SIMPLELEXER_FREE free
 #endif
 
 #ifndef SIMPLELEXER_DA_INIT_CAP
 #define SIMPLELEXER_DA_INIT_CAP 256
 #endif
 
 #define simplelexer_da_append_many(da, new_items, new_items_count)                              \
     do {                                                                                        \
         if ((da)->count + (new_items_count) > (da)->capacity) {                                 \
             if ((da)->capacity == 0) {                                                          \
                 (da)->capacity = SIMPLELEXER_DA_INIT_CAP;                                       \
             }                                                                                   \
             while ((da)->count + (new_items_count) > (da)->capacity) {                          \
                 (da)->capacity *= 2;                                                            \
             }                                                                                   \
             (da)->items = SIMPLELEXER_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));\
             SIMPLELEXER_ASSERT((da)->items != NULL && "Memory allocation failed");              \
         }                                                                                       \
         memcpy((da)->items + (da)->count, (new_items), (new_items_count) * sizeof(*(da)->items));\
         (da)->count += (new_items_count);                                                       \
     } while (0)
 
 #define simplelexer_sb_append_cstr(sb, cstr)  \
     do {                                      \
         const char *s = (cstr);               \
         size_t n = strlen(s);                 \
         simplelexer_da_append_many(sb, s, n);   \
     } while (0)
 
 #define simplelexer_sb_append_null(sb) simplelexer_da_append_many(sb, "", 1)
 
 typedef struct {
     const char *file_path;
     size_t row;
     size_t col;
 } SimpleLexer_Loc;
 
 #define SimpleLexer_Loc_Fmt "%s:%zu:%zu"
 #define SimpleLexer_Loc_Arg(loc) (loc).file_path, (loc).row, (loc).col
 
 #define SIMPLELEXER_LOW32(x) (((uint64_t)(x)) & 0xFFFFFFFF)
 #define SIMPLELEXER_ID(kind, index) ((SIMPLELEXER_LOW32(index) << 32) | (SIMPLELEXER_LOW32(kind)))
 #define SIMPLELEXER_KIND(id) SIMPLELEXER_LOW32(id)
 #define SIMPLELEXER_INDEX(id) SIMPLELEXER_LOW32((id) >> 32)
 
 typedef enum {
     SIMPLELEXER_INVALID,
     SIMPLELEXER_END,
     SIMPLELEXER_INT,
     SIMPLELEXER_SYMBOL,
     SIMPLELEXER_KEYWORD,
     SIMPLELEXER_PUNCT,
     SIMPLELEXER_STRING,
     SIMPLELEXER_COUNT_KINDS,
 } SimpleLexer_Kind;
 
 #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
 _Static_assert(SIMPLELEXER_COUNT_KINDS == 7, "Token kinds count mismatch");
 #else
 typedef char static_assertion_SIMPLELEXER_COUNT_KINDS[(SIMPLELEXER_COUNT_KINDS == 7) ? 1 : -1];
 #endif
 
 extern const char *simplelexer_kind_names[SIMPLELEXER_COUNT_KINDS];
 #define simplelexer_kind_name(kind) (SIMPLELEXER_ASSERT((uint64_t)kind < SIMPLELEXER_COUNT_KINDS), simplelexer_kind_names[(uint64_t)kind])
 
 typedef struct {
     uint64_t id;
     SimpleLexer_Loc loc;
     const char *begin;
     const char *end;
     long int_value;
 } SimpleLexer_Token;
 
 bool simplelexer_token_text_equal(SimpleLexer_Token a, SimpleLexer_Token b);
 bool simplelexer_token_text_equal_cstr(SimpleLexer_Token a, const char *b);
 
 #define SimpleLexer_Token_Fmt "%.*s"
 #define SimpleLexer_Token_Arg(t) (int)((t).end - (t).begin), (t).begin
 
 typedef struct {
     const char *opening;
     const char *closing;
 } SimpleLexer_MLComments;
 
 typedef struct {
     size_t cur;
     size_t bol;
     size_t row;
 } SimpleLexer_State;
 
 typedef struct {
     const char *file_path;
     const char *content;
     size_t size;
 
     size_t cur;
     size_t bol;
     size_t row;
 
     const char **puncts;
     size_t puncts_count;
     const char **keywords;
     size_t keywords_count;
     const char **sl_comments;
     size_t sl_comments_count;
     SimpleLexer_MLComments *ml_comments;
     size_t ml_comments_count;
 
     bool (*is_symbol_start)(char x);
     bool (*is_symbol)(char x);
     void (*diagf)(SimpleLexer_Loc loc, const char *level, const char *fmt, ...);
 } SimpleLexer;
 
 SimpleLexer simplelexer_create(const char *file_path, const char *content, size_t size);
 bool simplelexer_get_token(SimpleLexer *l, SimpleLexer_Token *t);
 SimpleLexer_State simplelexer_save(SimpleLexer *l);
 void simplelexer_rewind(SimpleLexer *l, SimpleLexer_State s);
 bool simplelexer_chop_char(SimpleLexer *l);
 void simplelexer_chop_chars(SimpleLexer *l, size_t n);
 void simplelexer_trim_left_ws(SimpleLexer *l);
 void simplelexer_drop_until_endline(SimpleLexer *l);
 SimpleLexer_Loc simplelexer_loc(SimpleLexer *l);
 bool simplelexer_default_is_symbol(char x);
 bool simplelexer_default_is_symbol_start(char x);
 void simplelexer_default_diagf(SimpleLexer_Loc loc, const char *level, const char *fmt, ...);
 void simplelexer_ignore_diagf(SimpleLexer_Loc loc, const char *level, const char *fmt, ...);
 bool simplelexer_expect_id(SimpleLexer *l, SimpleLexer_Token t, uint64_t id);
 bool simplelexer_expect_one_of_ids(SimpleLexer *l, SimpleLexer_Token t, uint64_t *ids, size_t ids_count);
 
 #endif /* SIMPLE_LEXER_H_ */
 