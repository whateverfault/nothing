#ifndef NOTHING_H
#define NOTHING_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#define NOTHING_DA_INIT_CAP 1024

#ifndef NOTHING_MALLOC
#include <stdlib.h>
#define NOTHING_MALLOC malloc
#endif

#ifndef NOTHING_CALLOC
#include <stdlib.h>
#define NOTHING_CALLOC calloc
#endif

#ifndef NOTHING_REALLOC
#include <stdlib.h>
#define NOTHING_REALLOC realloc
#endif

#ifndef NOTHING_FREE
#include <stdlib.h>
#define NOTHING_FREE free
#endif

#ifndef NOTHING_ASSERT
#include <assert.h>
#include <stdbool.h>
#define NOTHING_ASSERT assert
#endif

#define da_reserve(da, expected_capacity)                                                      \
    do {                                                                                       \
        if ((expected_capacity) > (da)->capacity) {                                            \
            if ((da)->capacity == 0) {                                                         \
                (da)->capacity = NOTHING_DA_INIT_CAP;                                          \
            }                                                                                  \
            while ((expected_capacity) > (da)->capacity) {                                     \
                (da)->capacity *= 2;                                                           \
            }                                                                                  \
            (da)->items = NOTHING_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            NOTHING_ASSERT((da)->items != NULL && "Buy more RAM lol");                         \
        }                                                                                      \
    } while (0)

#define da_resize(da, new_size)        \
    do {                               \
        da_reserve((da), (new_size));  \
        (da)->capacity = (new_size);   \
    } while (0)

#define da_append(da, item)                    \
    do {                                       \
        da_reserve(da, (da)->count+1);         \
        (da)->items[(da)->count++] = (item);   \
    } while (0)

#define da_append_many(da_dest, da_source)                  \
    do {                                                    \
        for (size_t i_ = 0; i_ < (da_source)->count; ++i_){ \
            da_append((da_dest), (da_source)->items[i_]);   \
        }                                                   \
    } while (0)

#define da_remove_unordered(da, i)                   \
    do {                                             \
        size_t j_ = (i);                              \
        (da)->items[j_] = (da)->items[--(da)->count]; \
    } while (0)

#define da_remove(da, i)                             \
    do {                                             \
        size_t j_ = (i);                             \
        NOTHING_ASSERT(j_ >= 0 && j_ < (da)->count); \
        for (; j_ < (da)->count - 1; ++j_){          \
            (da)->items[j_] = (da)->items[j_+1];     \
        }                                            \
        --(da)->count;                               \
    } while (0)

#define da_free(da) NOTHING_FREE((da).items)
#define da_pfree(da) NOTHING_FREE((da)->items)

#define sb_print(sb) printf("%.*s", (int)(sb).count, (sb).items)
#define sb_pprint(sb) printf("%.*s", (int)(sb)->count, (sb)->items)

#define CSTR_TO_SB(cstr) (StringBuilder){    \
.items=   (cstr),                            \
.count=   (sizeof(cstr)/sizeof((cstr)[0])-1),\
.capacity=(sizeof(cstr)/sizeof((cstr)[0])-1),\
}

#define CSTR_TO_SB_UNICODE(utf_8_str) (UnicodeStringBuilder){  \
.items=   (utf_8_str),                                         \
.count=   (sizeof(utf_8_str)/sizeof((utf_8_str)[0])),          \
.capacity=(sizeof(utf_8_str)/sizeof((utf_8_str)[0])),          \
}

typedef void (*free_func_t)(void *ptr);

typedef struct tnode TNode;
typedef struct tnode {
    TNode* parent;
    TNode* left;
    TNode* right;
    void* data;
} TNode;

typedef struct{
    size_t count;
    char* items;
} StringView;

typedef struct{
    size_t capacity;
    size_t count;
    char* items;
} StringBuilder;

typedef struct{
    size_t hash;
    void* value;
    struct KeyValue* next;
} KeyValue;

typedef struct HashMap {
    KeyValue** buckets;
    size_t capacity;
    size_t count;
} HashMap;

TNode* create_node(void* data, TNode* parent);

char* sb_to_cstr(StringBuilder *sb);
char* sv_to_cstr(StringView sv);
void sv_to_sb(StringView *sv, StringBuilder *sb);
void sv_to_escaped_sb(StringBuilder *sb, StringView *sv);
void sv_from_sb(StringView *sv, StringBuilder *sb);

void sv_trim(StringView* sv);
void sv_trim_left(StringView* sv);
void sv_trim_right(StringView* sv);

StringBuilder sb_trim(StringBuilder* sb);
StringBuilder sb_trim_left(StringBuilder* sb);
StringBuilder sb_trim_right(StringBuilder* sb);

bool sb_cmp_cstr(StringBuilder *sv, char *cstr);
bool sv_cmp_cstr(StringView *sv, char *cstr);
bool sv_cmp_sb(StringView *sv, StringBuilder *sb);
bool sv_cmp(StringView *sv_1, StringView *sv_2);

StringBuilder cstr_to_sb(char *cstr);
StringBuilder *sb_alloc(void);
StringBuilder *sb_new(char *s);
StringBuilder sb_cpy(StringBuilder sb);
int read_entire_file(StringBuilder* sb, const char* path);
int get_line_from_sb(StringBuilder* sb, StringView* sv, size_t line);
bool sb_getline(StringBuilder* sb, FILE *stream);
int sb_appendf(StringBuilder* sb, const char* fmt, ...);
int sb_append_sb(StringBuilder* sb_1, StringBuilder *sb_2);
int sb_append_sv(StringBuilder* sb, StringView *sv);
void sb_appendc(StringBuilder* sb, char c);
void sb_insertc(StringBuilder* sb, char c, size_t pos);

int is_all_digits_16(const char *p);
uint64_t parse_u64_fast(const char *p, size_t len);
uint64_t parse_u64_8(const char *p);
size_t scan_digits_simd(const char *p, size_t len);

bool parse_int_sv(StringView sv, int *out);
bool parse_float_sv(StringView sv, double *out);

HashMap* hm_alloc(void);
HashMap* hm_copy(HashMap *hm);
void hm_reset(HashMap* hm);
void hm_free(HashMap* hm);
int hm_put(HashMap* hm, char* key, void* value);
int hm_nput(HashMap* hm, char* key, size_t key_len, void* value);
int hm_put_sb(HashMap* hm, StringBuilder *sb, void* value);
int hm_put_hashed(HashMap* hm, size_t hash, void* value);

void *hm_get_hashed(HashMap* hm, size_t hash);
void* hm_get(HashMap* hm, const char* key);
void* hm_nget(HashMap* hm, const char* key, size_t key_len);

size_t hash_key(unsigned char* key);
size_t hash_nkey(unsigned char* key, size_t key_len);
size_t hash_sb(StringBuilder *sb);
size_t hash_combine(size_t a, size_t b);

size_t hash_ascii(char *str, size_t len);
size_t hash_utf32(int32_t *str, size_t len);

typedef struct{
    size_t capacity;
    size_t count;
    int32_t *items;
} UnicodeStringView;

typedef struct{
    size_t capacity;
    size_t count;
    int32_t *items;
} UnicodeStringBuilder;

UnicodeStringBuilder *sb_alloc_unicode();

void sv_to_unicode_sb(StringView *sv, UnicodeStringBuilder *sb);
void sv_to_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb);

bool sb_cmp_utf8(UnicodeStringBuilder *sv, char *cstr);
bool sv_cmp_utf8(UnicodeStringView *sv, char *cstr);
bool sv_cmp_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb);
bool sv_cmp_unicode_sb(StringView *sv, UnicodeStringBuilder *sb);
bool sv_cmp_unicode(UnicodeStringView *sv1, UnicodeStringView *sv2);

void sv_to_escaped_unicode_sb(UnicodeStringBuilder *sb, StringView *sv);
void sv_from_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb);

void sb_from_unicode_sb(StringBuilder *sb, UnicodeStringBuilder *sb_unicode);

int sb_appendf_unicode(UnicodeStringBuilder* sb, const char* fmt, ...);
int sb_append_sb_unicode(UnicodeStringBuilder* sb1, UnicodeStringBuilder *sb2);
int sb_append_sv_unicode(UnicodeStringBuilder* sb, UnicodeStringView *sv);
int sb_unicode_append_sv(UnicodeStringBuilder* sb, StringView *sv);
void sb_appendc_unicode(UnicodeStringBuilder* sb, int32_t c);
void sb_unicode_appendc(UnicodeStringBuilder* sb, char cp);

int hm_put_sb_unicode(HashMap* hm, UnicodeStringBuilder *sb, void* value);

void* hm_nget_unicode(HashMap* hm, int32_t* key, size_t key_len);

size_t hash_sb_unicode(UnicodeStringBuilder *sb);

size_t sb_print_unicode(UnicodeStringBuilder *sb);
size_t sb_printc_unicode(int32_t cp);

#endif //NOTHING_H

#ifdef NOTHING_IMPLEMENTATION

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <emmintrin.h>

TNode* create_node(void* data, TNode* parent) {
    TNode* node = (TNode*)NOTHING_MALLOC(sizeof(TNode));
    node->data = data;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void sv_from_sb(StringView *sv, StringBuilder *sb){
    sv->items = sb->items;
    sv->count = sb->count;
}

void sv_trim(StringView* sv){
    sv_trim_left(sv);
    sv_trim_right(sv);
}

void sv_trim_left(StringView* sv){
    while (sv->count > 0 && isspace(sv->items[0])){
        sv->items = ++sv->items;
        --sv->count;
    }
}

void sv_trim_right(StringView* sv){
    while (sv->count > 0 && isspace(sv->items[sv->count - 1])){
        --sv->count;
    }
}

StringBuilder sb_trim(StringBuilder* sb){
    StringView sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim(&sv);

    StringBuilder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

StringBuilder sb_trim_left(StringBuilder* sb){
    StringView sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim_left(&sv);

    StringBuilder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

StringBuilder sb_trim_right(StringBuilder* sb){
    StringView sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim_right(&sv);

    StringBuilder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

char* sv_to_cstr(StringView sv){
    char* cstr = NOTHING_MALLOC(sizeof(char)*(sv.count+1));
    for (size_t i = 0; i < sv.count; ++i){
        cstr[i] = sv.items[i];
    }
    
    cstr[sv.count] = 0;
    return cstr;
}

void sv_to_sb(StringView *sv, StringBuilder *sb){
    char* s = NOTHING_MALLOC(sizeof(char)*(sv->count));
    for (size_t i = 0; i < sv->count; ++i){
        s[i] = sv->items[i];
    }

    sb->items = s;
    sb->count = sv->count;
    sb->capacity = sv->count;
}

void sv_to_escaped_sb(StringBuilder *sb, StringView *sv) {
    for (size_t i = 0; i < sv->count; ++i) {
        if (sv->items[i] == '\\' && i + 1 < sv->count) {
            ++i;
            
            switch (sv->items[i]) {
                case 'n': {
                    sb_appendc(sb, '\n');
                    continue;
                } break;
                    
                case 'r': {
                    sb_appendc(sb, '\r');
                    continue;
                } break;
                    
                case 't': {
                    sb_appendc(sb, '\t');
                    continue;
                } break;
                    
                case 'b': {
                    sb_appendc(sb, '\b');
                    continue;
                } break;
                    
                case 'a': {
                    sb_appendc(sb, '\a');
                    continue;
                } break;
                    
                case '\'': {
                    sb_appendc(sb, '\'');
                    continue;
                } break;
                    
                case '\"': {
                    sb_appendc(sb, '\"');
                    continue;
                } break;
                    
                case '0': {
                    sb_appendc(sb, '\0');
                    continue;
                } break;
            }
        }
        
        sb_appendc(sb, sv->items[i]);
    }
}

bool sb_cmp_cstr(StringBuilder *sb, char *cstr) {
    size_t cstr_len = strlen(cstr);
    return cstr_len == sb->count && strncmp(sb->items, cstr, sb->count) == 0;
}

bool sv_cmp_cstr(StringView *sv, char *cstr) {
    size_t cstr_len = strlen(cstr);
    return cstr_len == sv->count && strncmp(sv->items, cstr, sv->count) == 0;
}

bool sv_cmp_sb(StringView *sv, StringBuilder *sb) {
    return sv->count == sb->count && strncmp(sv->items, sb->items, sv->count) == 0;
}

bool sv_cmp(StringView *sv_1, StringView *sv_2) {
    return sv_1->count == sv_2->count && strncmp(sv_1->items, sv_2->items, sv_1->count) == 0;
}

char* sb_to_cstr(StringBuilder *sb){
    char* cstr = NOTHING_MALLOC(sizeof(char)*(sb->count+1));
    for (size_t i = 0; i < sb->count; ++i){
        cstr[i] = sb->items[i];
    }
    
    cstr[sb->count] = 0;
    return cstr;
}

StringBuilder cstr_to_sb(char *cstr){
    size_t len = strlen(cstr);
    char* s = NOTHING_MALLOC(sizeof(char)*len);
    for (size_t i = 0; i < len; ++i){
        s[i] = cstr[i];
    }
    
    StringBuilder sb = {
        .items = s,
        .count = len,
        .capacity = len,
    };
    return sb;
}

StringBuilder *sb_alloc(void) {
    StringBuilder *sb = (StringBuilder*)NOTHING_MALLOC(sizeof(StringBuilder));
    *sb = (StringBuilder){0};
    return sb;
}

StringBuilder *sb_new(char *s) {
    size_t len = strlen(s);
    
    StringBuilder *sb = (StringBuilder*)NOTHING_MALLOC(sizeof(StringBuilder));
    *sb = (StringBuilder){
        .items = s,
        .count = len,
        .capacity = len,
    };
    
    return sb;
}

StringBuilder sb_cpy(StringBuilder sb) {
    char* s = NOTHING_MALLOC(sizeof(char)*sb.count);
    for (size_t i = 0; i < sb.count; ++i){
        s[i] = sb.items[i];
    }
    
    return (StringBuilder){
        .items = s,
        .count = sb.count,
        .capacity = sb.count,
    };
}

int get_line_from_sb(StringBuilder* sb, StringView* sv, size_t line){
    size_t start = 0;
    sv->count = 0;

    while (line > 0){
        if (start >= sb->count) return 0;
        if (sb->items[start++] == '\n'){
            --line;
        }
    }

    sv->items = sb->items+start;
    while (sv->items[sv->count] != '\n'){
        if (start+sv->count >= sb->count) return 0;
        ++sv->count;
    }
    
    return 1;
}

bool sb_getline(StringBuilder* sb, FILE *stream){
    if (stream == NULL) {
        return false;
    }
    
    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\n') {
            break;
        }
        
        sb_appendc(sb, c);
    }

    return true;
}

int sb_appendf(StringBuilder* sb, const char* fmt, ...){
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    da_reserve(sb, sb->count + n + 1);
    char *dest = sb->items + sb->count;
    va_start(args, fmt);
    vsnprintf(dest, n+1, fmt, args);
    va_end(args);

    sb->count += n;
    return n;
}

int sb_append_sb(StringBuilder* sb_1, StringBuilder *sb_2){
    if (sb_2->count == 0) {
        return 0;
    }
    
    da_reserve(sb_1, sb_1->count + sb_2->count);
    size_t start = sb_1->count;
    
    for (size_t i = 0; i < sb_2->count; ++i) {
        sb_1->items[sb_1->count++] = sb_2->items[i];
    }
    
    return sb_1->count - start;
}

int sb_append_sv(StringBuilder* sb, StringView *sv){
    if (sv->count == 0) {
        return 0;
    }
    
    da_reserve(sb, sb->count + sv->count);
    size_t start = sb->count;
    
    for (size_t i = 0; i < sv->count; ++i) {
        sb->items[sb->count++] = sv->items[i];
    }
    
    return sb->count - start;
}

void sb_appendc(StringBuilder* sb, char c){
    if (sb->count+1 >= sb->capacity){
        size_t expected = sb->capacity == 0? NOTHING_DA_INIT_CAP : sb->capacity;
        da_resize(sb, expected*2);
    }
    
    sb->items[sb->count++] = c;
}

void sb_insertc(StringBuilder* sb, char c, size_t pos){
    NOTHING_ASSERT(pos <= sb->count);
    
    if (sb->count+1 >= sb->capacity){
        size_t expected = sb->capacity == 0? NOTHING_DA_INIT_CAP : sb->capacity;
        da_resize(sb, expected*2);
    }

    for (size_t i = pos; i < sb->count - 1; ++i) {
        sb->items[i + 1] = sb->items[i];
    }
    
    sb->items[pos] = c;
    ++sb->count;
}

int is_all_digits_16(const char *p) {
    __m128i v = _mm_loadu_si128((const __m128i*)p);

    __m128i zero = _mm_set1_epi8('0');
    __m128i nine = _mm_set1_epi8('9');

    __m128i lt0 = _mm_cmplt_epi8(v, zero);
    __m128i gt9 = _mm_cmpgt_epi8(v, nine);

    __m128i bad = _mm_or_si128(lt0, gt9);

    return _mm_movemask_epi8(bad) == 0;
}

uint64_t parse_u64_fast(const char *p, size_t len) {
    uint64_t val = 0;

    for (size_t i = 0; i < len; i++) {
        val = val * 10 + (uint64_t)(p[i] - '0');
    }

    return val;
}

uint64_t parse_u64_8(const char *p) {
    return
        (uint64_t)(p[0]-'0') * 10000000ULL +
        (uint64_t)(p[1]-'0') * 1000000ULL +
        (uint64_t)(p[2]-'0') * 100000ULL +
        (uint64_t)(p[3]-'0') * 10000ULL +
        (uint64_t)(p[4]-'0') * 1000ULL +
        (uint64_t)(p[5]-'0') * 100ULL +
        (uint64_t)(p[6]-'0') * 10ULL +
        (uint64_t)(p[7]-'0');
}

size_t scan_digits_simd(const char *p, size_t len) {
    size_t i = 0;

    while (i + 16 <= len) {
        if (!is_all_digits_16(p + i)) break;
        i += 16;
    }

    while (i < len && p[i] >= '0' && p[i] <= '9') {
        i++;
    }

    return i;
}

bool parse_int_sv(StringView sv, int *out) {
    const char *p = sv.items;
    size_t len = sv.count;

    if (len == 0) return false;

    int sign = 1;
    if (*p == '-' || *p == '+') {
        if (*p == '-') sign = -1;
        p++;
        len--;
    }

    if (len == 0) return false;
    
    size_t digits = scan_digits_simd(p, len);

    if (digits != len) return false;
    if (digits > 18) return false;

    uint64_t val = parse_u64_fast(p, digits);

    *out = (long long)(sign * (long long)val);
    return true;
}

static const double pow10_inv[] = {
    1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7,
    1e-8, 1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15
};

bool parse_float_sv(StringView sv, double *out) {
    const char *p = sv.items;
    const char *end = p + sv.count;

    if (p == end) return false;
    
    int sign = 1;
    if (*p == '+' || *p == '-') {
        if (*p == '-') sign = -1;
        ++p;
        if (p == end) return false;
    }
    
    uint64_t mant = 0;
    int extra_exp = 0;
    int mant_digits = 0;
    int frac_digits = 0;
    bool seen_dot = false;
    bool has_digits = false;

    while (p < end) {
        char c = *p;

        if (c >= '0' && c <= '9') {
            has_digits = true;

            if (mant_digits < 19) {
                mant = mant * 10 + (c - '0');
                ++mant_digits;
                if (seen_dot) ++frac_digits;
            } else {
                if (!seen_dot) ++extra_exp;
            }
            ++p;
        }
        else if (c == '.') {
            if (seen_dot) return false;
            seen_dot = true;
            ++p;
        }
        else {
            break;
        }
    }

    if (!has_digits) return false;
    
    int exp10 = extra_exp - frac_digits;

    if (p < end && (*p == 'e' || *p == 'E')) {
        ++p;
        if (p == end) return false;

        int exp_sign = 1;
        if (*p == '+' || *p == '-') {
            if (*p == '-') exp_sign = -1;
            ++p;
            if (p == end) return false;
        }

        int exp_val = 0;
        bool has_exp_digits = false;

        while (p < end && *p >= '0' && *p <= '9') {
            has_exp_digits = true;

            if (exp_val < 10000) {
                exp_val = exp_val * 10 + (*p - '0');
            }
            p++;
        }

        if (!has_exp_digits) return false;

        exp10 += exp_sign * exp_val;
    }

    
    if (p != end) return false;
    double result = (double)mant;
    
    if (exp10 >= 0 && exp10 <= 15) {
        uint64_t scaled = mant;
    
        for (int i = 0; i < exp10; i++) {
            if (scaled > UINT64_MAX / 10) {
                result = (double)mant * pow(10.0, exp10);
                goto done;
            }
            scaled *= 10;
        }

        result = (double)scaled;
    }
    else if (exp10 >= -15 && exp10 < 0) {
        result = (double)mant * pow10_inv[-exp10];
    }
    else {
        result = (double)mant * pow(10.0, exp10);
    }

    done:
    *out = sign * result;
    return true;
}

int read_entire_file(StringBuilder* sb, const char* path){
    FILE* f = fopen(path, "rb");
    size_t new_count = 0;
    long long m = 0;
    
    if (f == NULL) return 0;
    if (fseek(f, 0, SEEK_END) < 0){
        fclose(f);
        return 0;
    }
    
#ifndef _WIN32
    m = ftell(f);
#else
    m = _ftelli64(f);
#endif
    
    if (m < 0 || fseek(f, 0, SEEK_SET) < 0) {
        fclose(f);
        return 0;
    }
    
    new_count = sb->count + m;
    if (new_count >= sb->capacity){
        sb->items = NOTHING_REALLOC(sb->items, new_count);
        NOTHING_ASSERT(sb->items != NULL && "Buy more RAM");
        sb->capacity = new_count;
    }

    fread(sb->items + sb->count, m, 1, f);
    if (ferror(f)) {
        fclose(f);
        return 0;
    }

    sb->count = new_count;
    fclose(f);
    return 1;
}

HashMap* hm_alloc(void){
    HashMap* hm = NOTHING_MALLOC(sizeof(HashMap));
    if (hm == NULL){
        return NULL;
    }

    hm->buckets = NOTHING_CALLOC(NOTHING_DA_INIT_CAP, sizeof(KeyValue*));
    if (hm->buckets == NULL){
        free(hm);
        return 0;
    }
    
    hm->capacity = NOTHING_DA_INIT_CAP;
    hm->count = 0;
    return hm;
}

HashMap* hm_copy(HashMap *hm) {
    HashMap *copy = hm_alloc();
    if (copy == NULL) {
        return copy;
    }

    copy->capacity = hm->capacity;
    copy->count = hm->count;
    
    copy->buckets = NOTHING_CALLOC(hm->capacity, sizeof(KeyValue*));

    KeyValue* cur = NULL;
    for (size_t i = 0; i < hm->capacity; ++i){
        KeyValue* source = hm->buckets[i];
        KeyValue** dest = &copy->buckets[i];
        if (source == NULL) continue;

        cur = source;
        while (cur != NULL) {
            *dest = NOTHING_MALLOC(sizeof(KeyValue));
            (*dest)->hash = cur->hash;
            (*dest)->value = cur->value;
            (*dest)->next = NULL;
            dest = (void*)&((*dest)->next);
            cur = (void*)cur->next;
        }
    }
    
    return copy;
}

void hm_free_buckets(HashMap* hm){
    KeyValue* cur = NULL;
    KeyValue* tmp = NULL;
    
    for (size_t i = 0; i < hm->capacity; ++i){
        KeyValue* bucket = hm->buckets[i];
        if (bucket == NULL) continue;
        
        cur = bucket;
        while (cur != NULL) {
            tmp = cur;
            cur = (KeyValue*)cur->next;
            
            free(tmp);
        }
    }
}

void hm_reset(HashMap* hm){
    hm_free_buckets(hm);
}

void hm_free(HashMap* hm){
    hm_free_buckets(hm);
    free(hm->buckets);
    free(hm);
}

int hm_put_hashed(HashMap* hm, size_t hash, void* value) {
    size_t index = hash % hm->capacity;
    
    KeyValue* cur = hm->buckets[index];
    
    KeyValue* prev = NULL;
    
    while (cur != NULL){
        if (cur->hash == hash){
            cur->value = value;
            return 0;
        }
        
        prev = cur;
        cur = (KeyValue*)cur->next;
    }
    
    KeyValue* new = NOTHING_MALLOC(sizeof(KeyValue));
    if (new == NULL){
        return 1;
    }
    
    new->hash = hash;
    new->value = value;
    new->next = NULL;
    
    if (prev == NULL){
        hm->buckets[index] = new;
    } else {
        prev->next = (struct KeyValue *)new;
    }
    
    ++hm->count;

    return 0;
}

int hm_put(HashMap* hm, char* key, void* value){
    if (key == NULL) {
        return 1;
    }
    
    return hm_put_hashed(hm, hash_key((void*)key), value);
}

char *__strndup(const char *s, size_t n) {
    char *result = (char *)malloc(n + 1);
    if (!result) return NULL;
    strncpy(result, s, n);
    result[n] = '\0';
    return result;
}

int hm_nput(HashMap* hm, char* key, size_t key_len, void* value){
    if (key == NULL) {
        return 1;
    }
    
    return hm_put_hashed(hm, hash_nkey((void*)key, key_len), value);
}

int hm_put_sb(HashMap* hm, StringBuilder *sb, void* value) {
    return hm_nput(hm, sb->items, sb->count, value);
}

void *hm_get_hashed(HashMap* hm, size_t hash) {
    if (hm->count == 0) {
        return NULL;
    }
    
    size_t index = hash % hm->capacity;
    KeyValue* cur = hm->buckets[index];
    
    while (cur != NULL){
        if (cur->hash == hash) return cur->value;
        cur = (KeyValue*)cur->next;
    }
    
    return NULL;
}

void* hm_get(HashMap* hm, const char* key) {
    return hm_get_hashed(hm, hash_key((void*)key));
}

void* hm_nget(HashMap* hm, const char* key, size_t key_len) {
    return hm_get_hashed(hm, hash_nkey((void*)key, key_len));
}

size_t hash_key(unsigned char* key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

size_t hash_nkey(unsigned char* key, size_t key_len) {
    unsigned long hash = 5381;
    size_t pos = 0;
    while (pos < key_len) {
        hash = ((hash << 5) + hash) + key[pos];
        ++pos;
    }
    return hash;
}

size_t hash_sb(StringBuilder *sb) {
    return hash_nkey((unsigned char*)sb->items, sb->count);
}

size_t hash_combine(size_t a, size_t b) {
    return a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2));
}

#ifdef NOTHING_UNICODE

#ifndef NOTHING_UTF8_H_PATH
#define NOTHING_UTF8_H_PATH "utf8.h"
#endif // NOTHING_UTF8_H_PATH

#include NOTHING_UTF8_H_PATH

#ifndef NOTHING_SB_STACK_BUF
#define NOTHING_SB_STACK_BUF 512
#endif

UnicodeStringBuilder *sb_alloc_unicode() {
    UnicodeStringBuilder *sb = (UnicodeStringBuilder*)NOTHING_MALLOC(sizeof(UnicodeStringBuilder));
    *sb = (UnicodeStringBuilder){0};
    return sb;
}

void sv_to_unicode_sb(StringView *sv, UnicodeStringBuilder *sb) {
    utf8_int32_t* s = NOTHING_MALLOC(sizeof(utf8_int32_t)*(sv->count));
    
    const char *p = sv->items;
    utf8_int32_t cp;
    size_t i = 0;

    while (*p) {
        if (i >= sv->count) {
            break;
        }

        p = utf8codepoint(p, &cp);
        s[i] = cp;

        i++;
    }

    sb->items = s;
    sb->count = sv->count;
    sb->capacity = sv->count;
}

void sv_to_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb) {
    utf8_int32_t* s = NOTHING_MALLOC(sizeof(utf8_int32_t)*(sv->count));
    for (size_t i = 0; i < sv->count; ++i){
        s[i] = sv->items[i];
    }

    sb->items = s;
    sb->count = sv->count;
    sb->capacity = sv->count;
}

bool sb_cmp_utf8(UnicodeStringBuilder *sb, char *cstr) {
    const char *p = cstr;
    utf8_int32_t cp;
    size_t i = 0;

    while (*p) {
        if (i >= sb->count) {
            return false;
        }

        p = utf8codepoint(p, &cp);

        if (sb->items[i] != cp) {
            return false;
        }

        i++;
    }

    return i == sb->count;
}

bool sv_cmp_utf8(UnicodeStringView *sv, char *cstr) {
    const char *p = cstr;
    utf8_int32_t cp;
    size_t i = 0;

    while (*p) {
        if (i >= sv->count) {
            return false;
        }

        p = utf8codepoint(p, &cp);

        if (sv->items[i] != cp) {
            return false;
        }

        i++;
    }

    return i == sv->count;
}

bool sv_cmp_unicode_sb(StringView *sv, UnicodeStringBuilder *sb) {
    if (utf8nlen(sv->items, sv->count) != sb->count) {
        return false;
    }

    const char *p = sv->items;
    utf8_int32_t cp;
    size_t i = 0;
    
    while (*p && i < sb->count) {
        p = utf8codepoint(p, &cp);

        if (sb->items[i] != cp) {
            return false;
        }

        i++;
    }

    return true;
}

bool sv_cmp_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb) {
    if (sv->count != sb->count) {
        return false;
    }

    for (size_t i = 0; i < sv->count; ++i) {
        if (sv->items[i] != sb->items[i]) {
            return false;
        }
    }

    return true;
}

bool sv_cmp_unicode(UnicodeStringView *sv1, UnicodeStringView *sv2) {
    if (sv1->count != sv2->count) {
        return false;
    }

    for (size_t i = 0; i < sv1->count; ++i) {
        if (sv1->items[i] != sv2->items[i]) {
            return false;
        }
    }

    return true;
}

void sv_to_escaped_unicode_sb(UnicodeStringBuilder *sb, StringView *sv) {
    const char *p = sv->items;
    utf8_int32_t cp;
    size_t i = 0;

    size_t len = utf8nlen(sv->items, sv->count);
    
    while (*p && i < len) {
        p = utf8codepoint(p, &cp);
        
        if (cp == '\\' && i + 1 < len) {
            ++i;
            
            switch (cp) {
                case 'n': {
                    sb_appendc_unicode(sb, '\n');
                    continue;
                } break;
                    
                case 'r': {
                    sb_appendc_unicode(sb, '\r');
                    continue;
                } break;
                    
                case 't': {
                    sb_appendc_unicode(sb, '\t');
                    continue;
                } break;
                    
                case 'b': {
                    sb_appendc_unicode(sb, '\b');
                    continue;
                } break;
                    
                case 'a': {
                    sb_appendc_unicode(sb, '\a');
                    continue;
                } break;
                    
                case '\'': {
                    sb_appendc_unicode(sb, '\'');
                    continue;
                } break;
                    
                case '\"': {
                    sb_appendc_unicode(sb, '\"');
                    continue;
                } break;
            }
        }
        
        sb_appendc_unicode(sb, cp);
        ++i;
    }
}

int utf8_len_for_codepoint(uint32_t cp) {
    if (cp <= 0x7F) return 1;
    if (cp <= 0x7FF) return 2;
    if (cp <= 0xFFFF) return 3;
    if (cp <= 0x10FFFF) return 4;
    return 0;
}

char* utf32_to_utf8(int32_t *in, size_t char_count) {
    size_t byte_len = 0;
    for (size_t i = 0; i < char_count; i++) {
        byte_len += utf8_len_for_codepoint((uint32_t)in[i]);
    }
    
    char *out = (char*)malloc(byte_len);
    if (!out) return NULL;

    char *p = out;
    for (size_t i = 0; i < char_count; i++) {
        uint32_t cp = (uint32_t)in[i];

        if (cp <= 0x7F) {
            *p++ = (char)cp;
        } else if (cp <= 0x7FF) {
            *p++ = (char)(0xC0 | (cp >> 6));
            *p++ = (char)(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            *p++ = (char)(0xE0 | (cp >> 12));
            *p++ = (char)(0x80 | ((cp >> 6) & 0x3F));
            *p++ = (char)(0x80 | (cp & 0x3F));
        } else if (cp <= 0x10FFFF) {
            *p++ = (char)(0xF0 | (cp >> 18));
            *p++ = (char)(0x80 | ((cp >> 12) & 0x3F));
            *p++ = (char)(0x80 | ((cp >> 6) & 0x3F));
            *p++ = (char)(0x80 | (cp & 0x3F));
        }
    }

    return out;
}

void sb_from_unicode_sb(StringBuilder *sb, UnicodeStringBuilder *sb_unicode) {
    size_t len = 0;
    for (size_t i = 0; i < sb_unicode->count; i++) {
        len += utf8_len_for_codepoint((uint32_t)sb_unicode->items[i]);
    }
    
    sb->items = utf32_to_utf8(sb_unicode->items, sb_unicode->count);
    sb->count = len;
    sb->capacity = len;
}

void sv_from_sb_unicode(UnicodeStringView *sv, UnicodeStringBuilder *sb){
    sv->items = sb->items;
    sv->count = sb->count;
}

int sb_appendf_unicode(UnicodeStringBuilder* sb, const char* fmt, ...){
    va_list args;

    char stackbuf[NOTHING_SB_STACK_BUF];
    
    va_start(args, fmt);
    int n = vsnprintf(stackbuf, sizeof(stackbuf), fmt, args);
    va_end(args);

    if (n < 0) return -1;

    const char *src = NULL;
    char *heapbuf = NULL;

    if ((size_t)n < sizeof(stackbuf)) {
        src = stackbuf;
    } else {
        heapbuf = (char*)malloc(n + 1);
        if (!heapbuf) return -1;

        va_start(args, fmt);
        vsnprintf(heapbuf, n + 1, fmt, args);
        va_end(args);

        src = heapbuf;
    }
    
    const char *p = src;
    utf8_int32_t cp;
    int appended = 0;

    while (*p) {
        p = utf8codepoint(p, &cp);

        if (sb->count >= sb->capacity) {
            size_t newcap = sb->capacity ? sb->capacity * 2 : 16;
            da_resize(sb, newcap);
        }

        sb->items[sb->count++] = cp;
        appended++;
    }

    if (heapbuf) free(heapbuf);

    return appended;
}

int sb_append_sb_unicode(UnicodeStringBuilder* dst, UnicodeStringBuilder *src){
    if (src->count == 0) return 0;

    da_reserve(dst, dst->count + src->count);

    size_t start = dst->count;

    for (size_t i = 0; i < src->count; ++i) {
        dst->items[dst->count++] = src->items[i];
    }

    return dst->count - start;
}

int sb_append_sv_unicode(UnicodeStringBuilder* sb, UnicodeStringView *sv){
    if (sv->count == 0) return 0;

    da_reserve(sb, sb->count + sv->count);

    size_t start = sb->count;

    for (size_t i = 0; i < sv->count; ++i) {
        sb->items[sb->count++] = sv->items[i];
    }

    return sb->count - start;
}

int sb_unicode_append_sv(UnicodeStringBuilder* sb, StringView *sv) {
    if (sv->count == 0) return 0;

    size_t len = utf8nlen(sv->items, sv->count);
    
    da_reserve(sb, sb->count + len);

    size_t start = sb->count;

    const char *p = sv->items;
    utf8_int32_t cp;
    size_t i = 0;
    
    while (*p) {
        if (i >= len) {
            break;
        }

        p = utf8codepoint(p, &cp);
        sb->items[sb->count++] = cp;
        
        i++;
    }

    return sb->count - start;
}

void sb_appendc_unicode(UnicodeStringBuilder* sb, int32_t cp){
    if (sb->count >= sb->capacity) {
        size_t newcap = sb->capacity ? sb->capacity * 2 : NOTHING_DA_INIT_CAP;
        da_resize(sb, newcap);
    }

    sb->items[sb->count++] = cp;
}

void sb_unicode_appendc(UnicodeStringBuilder* sb, char cp){
    if (sb->count >= sb->capacity) {
        size_t newcap = sb->capacity ? sb->capacity * 2 : NOTHING_DA_INIT_CAP;
        da_resize(sb, newcap);
    }

    sb->items[sb->count++] = (int32_t)(unsigned char)cp;
}

int sb_append_utf8(UnicodeStringBuilder* sb, const char *str) {
    const char *p = str;
    utf8_int32_t cp;
    int count = 0;

    while (*p) {
        p = utf8codepoint(p, &cp);
        sb_unicode_appendc(sb, cp);
        count++;
    }

    return count;
}

#define FNV_OFFSET_BASIS 14695981039346656037ULL
#define FNV_PRIME        1099511628211ULL

size_t hash_fold_codepoint(size_t hash, uint32_t codepoint) {
    for (int i = 0; i < 4; ++i) {
        hash ^= (codepoint & 0xFF);
        hash *= FNV_PRIME;
        codepoint >>= 8;
    }
    return hash;
}

size_t hash_ascii(char *str, size_t len) {
    size_t hash = FNV_OFFSET_BASIS;
    
    const char *p = str;
    const char *end = str + len;
    int32_t cp;

    while (p < end && *p != '\0') {
        p = utf8codepoint(p, &cp); 
        hash = hash_fold_codepoint(hash, (uint32_t)cp);
    }
    
    return hash;
}

size_t hash_utf32(int32_t *str, size_t len) {
    size_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < len; ++i) {
        hash = hash_fold_codepoint(hash, (uint32_t)str[i]);
    }
    return hash;
}

int hm_put_sb_unicode(HashMap* hm, UnicodeStringBuilder *sb, void* value) {
    size_t hash = hash_utf32(sb->items, sb->count);
    return hm_put_hashed(hm, hash, value);
}

size_t hash_sb_unicode(UnicodeStringBuilder *sb) {
    return hash_utf32(sb->items, sb->count);
}

void* hm_nget_unicode(HashMap* hm, int32_t* key, size_t key_len) {
    return hm_get_hashed(hm, hash_utf32(key, key_len));
}

int encode_utf8(uint32_t cp, char *out) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

size_t sb_print_unicode(UnicodeStringBuilder *sb) {
    char buffer[1024];
    size_t buf_idx = 0;

    for (size_t i = 0; i < sb->count; i++) {
        if (buf_idx > sizeof(buffer) - 4) {
            fwrite(buffer, 1, buf_idx, stdout);
            buf_idx = 0;
        }
        buf_idx += encode_utf8((uint32_t)sb->items[i], &buffer[buf_idx]);
    }

    if (buf_idx > 0) {
        fwrite(buffer, 1, buf_idx, stdout);
    }
}

size_t sb_printc_unicode(int32_t cp) {
    char buf[5] = {0};
    utf8catcodepoint(buf, cp, 5);
    printf("%s", buf);
}

#endif // NOTHING_UNICODE

#endif //NOTHING_IMPLEMENTATION