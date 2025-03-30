#define SIMPLELEXER_IMPLEMENTATION
#include <stdarg.h>

#include "simple-lexer.h"

const char *simplelexer_kind_names[SIMPLELEXER_COUNT_KINDS];

static void init_simplelexer_kind_names(void) {
    simplelexer_kind_names[SIMPLELEXER_INVALID] = "INVALID";
    simplelexer_kind_names[SIMPLELEXER_END]     = "END";
    simplelexer_kind_names[SIMPLELEXER_INT]     = "INT";
    simplelexer_kind_names[SIMPLELEXER_SYMBOL]  = "SYMBOL";
    simplelexer_kind_names[SIMPLELEXER_KEYWORD] = "KEYWORD";
    simplelexer_kind_names[SIMPLELEXER_PUNCT]   = "PUNCT";
    simplelexer_kind_names[SIMPLELEXER_STRING]  = "STRING";
}

SimpleLexer simplelexer_create(const char *file_path, const char *content, size_t size) {
    static bool inited = false;
    if (!inited) {
        init_simplelexer_kind_names();
        inited = true;
    }
    SimpleLexer l = {0};
    l.file_path = file_path;
    l.content = content;
    l.size = size;
    l.diagf = simplelexer_default_diagf;
    l.is_symbol = simplelexer_default_is_symbol;
    l.is_symbol_start = simplelexer_default_is_symbol_start;
    return l;
}

bool simplelexer_chop_char(SimpleLexer *l) {
    if (l->cur < l->size) {
        char x = l->content[l->cur++];
        if (x == '\n') {
            l->bol = l->cur;
            l->row++;
        }
        return true;
    }
    return false;
}

void simplelexer_chop_chars(SimpleLexer *l, size_t n) {
    while (n-- > 0 && simplelexer_chop_char(l));
}

void simplelexer_trim_left_ws(SimpleLexer *l) {
    while (l->cur < l->size && isspace(l->content[l->cur])) {
        simplelexer_chop_char(l);
    }
}

SimpleLexer_Loc simplelexer_loc(SimpleLexer *l) {
    SimpleLexer_Loc loc = { l->file_path, l->row + 1, l->cur - l->bol + 1 };
    return loc;
}

bool simplelexer_default_is_symbol(char x) {
    return isalnum(x) || x == '_';
}

bool simplelexer_default_is_symbol_start(char x) {
    return isalpha(x) || x == '_';
}

static bool simplelexer_starts_with(SimpleLexer *l, const char *prefix) {
    for (size_t i = 0; l->cur + i < l->size && prefix[i] != '\0'; ++i) {
        if (l->content[l->cur + i] != prefix[i])
            return false;
    }
    return true;
}

void simplelexer_drop_until_endline(SimpleLexer *l) {
    while (l->cur < l->size) {
        char x = l->content[l->cur];
        simplelexer_chop_char(l);
        if (x == '\n') break;
    }
}

static void simplelexer_chop_until(SimpleLexer *l, const char *prefix) {
    while (l->cur < l->size && !simplelexer_starts_with(l, prefix)) {
        simplelexer_chop_char(l);
    }
}

static void simplelexer_skip_comments(SimpleLexer *l) {
    while (l->cur < l->size) {
        simplelexer_trim_left_ws(l);
        bool skipped = false;

        for (size_t i = 0; i < l->sl_comments_count; i++) {
            if (simplelexer_starts_with(l, l->sl_comments[i])) {
                simplelexer_drop_until_endline(l);
                skipped = true;
                break;
            }
        }
        if (skipped) continue;

        for (size_t i = 0; i < l->ml_comments_count; i++) {
            if (simplelexer_starts_with(l, l->ml_comments[i].opening)) {
                simplelexer_chop_chars(l, strlen(l->ml_comments[i].opening));
                simplelexer_chop_until(l, l->ml_comments[i].closing);
                simplelexer_chop_chars(l, strlen(l->ml_comments[i].closing));
                skipped = true;
                break;
            }
        }
        if (!skipped) break;
    }
}

bool simplelexer_get_token(SimpleLexer *l, SimpleLexer_Token *t) {
    simplelexer_skip_comments(l);
    memset(t, 0, sizeof(*t));
    t->loc = simplelexer_loc(l);
    t->begin = &l->content[l->cur];
    t->end = t->begin;

    if (l->cur >= l->size) {
        t->id = SIMPLELEXER_END;
        return false;
    }

    for (size_t i = 0; i < l->puncts_count; i++) {
        if (simplelexer_starts_with(l, l->puncts[i])) {
            size_t n = strlen(l->puncts[i]);
            t->id = SIMPLELEXER_ID(SIMPLELEXER_PUNCT, i);
            t->end += n;
            simplelexer_chop_chars(l, n);
            return true;
        }
    }

    if (isdigit(l->content[l->cur])) {
        t->id = SIMPLELEXER_INT;
        while (l->cur < l->size && isdigit(l->content[l->cur])) {
            t->int_value = t->int_value * 10 + (l->content[l->cur] - '0');
            t->end++;
            simplelexer_chop_char(l);
        }
        return true;
    }

    if (l->is_symbol_start(l->content[l->cur])) {
        t->id = SIMPLELEXER_SYMBOL;
        while (l->cur < l->size && l->is_symbol(l->content[l->cur])) {
            t->end++;
            simplelexer_chop_char(l);
        }
        for (size_t i = 0; i < l->keywords_count; i++) {
            size_t n = strlen(l->keywords[i]);
            if (n == (size_t)(t->end - t->begin) &&
                memcmp(l->keywords[i], t->begin, n) == 0) {
                t->id = SIMPLELEXER_ID(SIMPLELEXER_KEYWORD, i);
                break;
            }
        }
        return true;
    }

    simplelexer_chop_char(l);
    t->end++;
    return false;
}

void simplelexer_sb_append_id_display(SimpleLexer_StringBuilder *sb, SimpleLexer *l, uint64_t id) {
    uint64_t kind = SIMPLELEXER_KIND(id);
    uint64_t index = SIMPLELEXER_INDEX(id);
    switch (kind) {
        case SIMPLELEXER_INVALID:
        case SIMPLELEXER_END:
        case SIMPLELEXER_STRING:
        case SIMPLELEXER_INT:
        case SIMPLELEXER_SYMBOL:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            break;
        case SIMPLELEXER_KEYWORD:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            simplelexer_sb_append_cstr(sb, " `");
            SIMPLELEXER_ASSERT(index < l->keywords_count);
            simplelexer_sb_append_cstr(sb, l->keywords[index]);
            simplelexer_sb_append_cstr(sb, "`");
            break;
        case SIMPLELEXER_PUNCT:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            simplelexer_sb_append_cstr(sb, " `");
            SIMPLELEXER_ASSERT(index < l->puncts_count);
            simplelexer_sb_append_cstr(sb, l->puncts[index]);
            simplelexer_sb_append_cstr(sb, "`");
            break;
        default:
            SIMPLELEXER_ASSERT(0 && "unreachable");
    }
}

void simplelexer_sb_append_token_display(SimpleLexer_StringBuilder *sb, SimpleLexer *l, SimpleLexer_Token t) {
    uint64_t kind = SIMPLELEXER_KIND(t.id);
    uint64_t index = SIMPLELEXER_INDEX(t.id);
    switch (kind) {
        case SIMPLELEXER_INVALID:
        case SIMPLELEXER_END:
        case SIMPLELEXER_STRING:
        case SIMPLELEXER_INT:
        case SIMPLELEXER_SYMBOL:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            simplelexer_sb_append_cstr(sb, " `");
            simplelexer_da_append_many(sb, t.begin, t.end - t.begin);
            simplelexer_sb_append_cstr(sb, "`");
            break;
        case SIMPLELEXER_KEYWORD:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            simplelexer_sb_append_cstr(sb, " `");
            SIMPLELEXER_ASSERT(index < l->keywords_count);
            simplelexer_sb_append_cstr(sb, l->keywords[index]);
            simplelexer_sb_append_cstr(sb, "`");
            break;
        case SIMPLELEXER_PUNCT:
            simplelexer_sb_append_cstr(sb, simplelexer_kind_name(kind));
            simplelexer_sb_append_cstr(sb, " `");
            SIMPLELEXER_ASSERT(index < l->puncts_count);
            simplelexer_sb_append_cstr(sb, l->puncts[index]);
            simplelexer_sb_append_cstr(sb, "`");
            break;
        default:
            SIMPLELEXER_ASSERT(0 && "unreachable");
    }
}

bool simplelexer_expect_id(SimpleLexer *l, SimpleLexer_Token t, uint64_t id) {
    return simplelexer_expect_one_of_ids(l, t, &id, 1);
}

bool simplelexer_expect_one_of_ids(SimpleLexer *l, SimpleLexer_Token t, uint64_t *ids, size_t ids_count) {
    bool result = false;
    SimpleLexer_StringBuilder sb = {0};
    for (size_t i = 0; i < ids_count; i++) {
        if (t.id == ids[i]) {
            simplelexer_return_defer(true);
        }
    }
    simplelexer_sb_append_cstr(&sb, "Expected ");
    for (size_t i = 0; i < ids_count; i++) {
        if (i > 0)
            simplelexer_sb_append_cstr(&sb, ", ");
        simplelexer_sb_append_id_display(&sb, l, ids[i]);
    }
    simplelexer_sb_append_cstr(&sb, " but got ");
    simplelexer_sb_append_token_display(&sb, l, t);
    simplelexer_sb_append_null(&sb);
    l->diagf(t.loc, "ERROR", "%s", sb.items);
defer:
    free(sb.items);
    return result;
}

void simplelexer_default_diagf(SimpleLexer_Loc loc, const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, SimpleLexer_Loc_Fmt ": %s: ", SimpleLexer_Loc_Arg(loc), level);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void simplelexer_ignore_diagf(SimpleLexer_Loc loc, const char *level, const char *fmt, ...) {
    (void) loc;
    (void) level;
    (void) fmt;
}

bool simplelexer_token_text_equal(SimpleLexer_Token a, SimpleLexer_Token b) {
    size_t na = a.end - a.begin;
    size_t nb = b.end - b.begin;
    if (na != nb) return false;
    return memcmp(a.begin, b.begin, na) == 0;
}

bool simplelexer_token_text_equal_cstr(SimpleLexer_Token a, const char *b) {
    size_t na = a.end - a.begin;
    size_t nb = strlen(b);
    if (na != nb) return false;
    return memcmp(a.begin, b, na) == 0;
}

SimpleLexer_State simplelexer_save(SimpleLexer *l) {
    SimpleLexer_State s = { l->cur, l->bol, l->row };
    return s;
}

void simplelexer_rewind(SimpleLexer *l, SimpleLexer_State s) {
    l->cur = s.cur;
    l->bol = s.bol;
    l->row = s.row;
}
