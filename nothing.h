#ifndef NOTHING_H
#define NOTHING_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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
} String_View;

typedef struct{
    size_t capacity;
    size_t count;
    char* items;
} String_Builder;

typedef struct{
    char* key;
    void* value;
    struct KeyValue* next;
} KeyValue;

typedef struct HashMap {
    KeyValue** buckets;
    size_t capacity;
    size_t count;
} HashMap;

TNode* create_node(void* data, TNode* parent);

char* sb_to_cstr(String_Builder *sb);
char* sv_to_cstr(String_View sv);
void sv_to_sb(String_View *sv, String_Builder *sb);
void sv_to_escaped_sb(String_Builder *sb, String_View *sv);
void sv_from_sb(String_View *sv, String_Builder *sb);

void sv_trim(String_View* sv);
void sv_trim_left(String_View* sv);
void sv_trim_right(String_View* sv);

String_Builder sb_trim(String_Builder* sb);
String_Builder sb_trim_left(String_Builder* sb);
String_Builder sb_trim_right(String_Builder* sb);

bool sv_cmp_cstr(String_View *sv, char *cstr);
bool sv_cmp_sb(String_View *sv, String_Builder *sb);
bool sv_cmp(String_View *sv_1, String_View *sv_2);
String_Builder cstr_to_sb(char *cstr);
String_Builder *sb_alloc(void);
String_Builder *sb_new(char *s);
String_Builder sb_cpy(String_Builder sb);
int read_entire_file(String_Builder* sb, const char* path);
int get_line_from_sb(String_Builder* sb, String_View* sv, size_t line);
bool sb_getline(String_Builder* sb, FILE *stream);
int sb_appendf(String_Builder* sb, const char* fmt, ...);
int sb_append_sb(String_Builder* sb_1, String_Builder *sb_2);
int sb_append_sv(String_Builder* sb, String_View *sv);
void sb_appendc(String_Builder* sb, char c);
void sb_insertc(String_Builder* sb, char c, size_t pos);

HashMap* hm_alloc(void);
HashMap* hm_copy(HashMap *hm);
void hm_reset(HashMap* hm);
void hm_free(HashMap* hm);
int hm_put(HashMap* hm, char* key, void* value);
int hm_nput(HashMap* hm, char* key, size_t key_len, void* value);
void* hm_get(HashMap* hm, const char* key);
void* hm_nget(HashMap* hm, const char* key, size_t key_len);
unsigned long hash_key(const unsigned char* key);
unsigned long hash_nkey(const unsigned char* key, size_t key_len);

#endif //NOTHING_H

#ifdef NOTHING_IMPLEMENTATION

#include <stdio.h>
#include <ctype.h>

TNode* create_node(void* data, TNode* parent) {
    TNode* node = (TNode*)NOTHING_MALLOC(sizeof(TNode));
    node->data = data;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void sv_from_sb(String_View *sv, String_Builder *sb){
    sv->items = sb->items;
    sv->count = sb->count;
}

void sv_trim(String_View* sv){
    sv_trim_left(sv);
    sv_trim_right(sv);
}

void sv_trim_left(String_View* sv){
    while (sv->count > 0 && isspace(sv->items[0])){
        sv->items = ++sv->items;
        --sv->count;
    }
}

void sv_trim_right(String_View* sv){
    while (sv->count > 0 && isspace(sv->items[sv->count - 1])){
        --sv->count;
    }
}

String_Builder sb_trim(String_Builder* sb){
    String_View sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim(&sv);

    String_Builder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

String_Builder sb_trim_left(String_Builder* sb){
    String_View sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim_left(&sv);

    String_Builder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

String_Builder sb_trim_right(String_Builder* sb){
    String_View sv = {0};
    sv_from_sb(&sv, sb);
    sv_trim_right(&sv);

    String_Builder trimmed = {0};
    sv_to_sb(&sv, &trimmed);
    return trimmed;
}

char* sv_to_cstr(String_View sv){
    char* cstr = NOTHING_MALLOC(sizeof(char)*(sv.count+1));
    for (size_t i = 0; i < sv.count; ++i){
        cstr[i] = sv.items[i];
    }
    
    cstr[sv.count] = 0;
    return cstr;
}

void sv_to_sb(String_View *sv, String_Builder *sb){
    char* s = NOTHING_MALLOC(sizeof(char)*(sv->count));
    for (size_t i = 0; i < sv->count; ++i){
        s[i] = sv->items[i];
    }

    sb->items = s;
    sb->count = sv->count;
    sb->capacity = sv->count;
}

void sv_to_escaped_sb(String_Builder *sb, String_View *sv) {
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

bool sv_cmp_cstr(String_View *sv, char *cstr) {
    size_t cstr_len = strlen(cstr);
    return cstr_len == sv->count && strncmp(sv->items, cstr, sv->count) == 0;
}

bool sv_cmp_sb(String_View *sv, String_Builder *sb) {
    return sv->count == sb->count && strncmp(sv->items, sb->items, sv->count) == 0;
}

bool sv_cmp(String_View *sv_1, String_View *sv_2) {
    return sv_1->count == sv_2->count && strncmp(sv_1->items, sv_2->items, sv_1->count) == 0;
}

char* sb_to_cstr(String_Builder *sb){
    char* cstr = NOTHING_MALLOC(sizeof(char)*(sb->count+1));
    for (size_t i = 0; i < sb->count; ++i){
        cstr[i] = sb->items[i];
    }
    
    cstr[sb->count] = 0;
    return cstr;
}

String_Builder cstr_to_sb(char *cstr){
    size_t len = strlen(cstr);
    char* s = NOTHING_MALLOC(sizeof(char)*len);
    for (size_t i = 0; i < len; ++i){
        s[i] = cstr[i];
    }
    
    String_Builder sb = {
        .items = s,
        .count = len,
        .capacity = len,
    };
    return sb;
}

String_Builder *sb_alloc(void) {
    String_Builder *sb = (String_Builder*)NOTHING_MALLOC(sizeof(String_Builder));
    *sb = (String_Builder){0};
    return sb;
}

String_Builder *sb_new(char *s) {
    size_t len = strlen(s);
    
    String_Builder *sb = (String_Builder*)NOTHING_MALLOC(sizeof(String_Builder));
    *sb = (String_Builder){
        .items = s,
        .count = len,
        .capacity = len,
    };
    
    return sb;
}

String_Builder sb_cpy(String_Builder sb) {
    char* s = NOTHING_MALLOC(sizeof(char)*sb.count);
    for (size_t i = 0; i < sb.count; ++i){
        s[i] = sb.items[i];
    }
    
    return (String_Builder){
        .items = s,
        .count = sb.count,
        .capacity = sb.count,
    };
}

int get_line_from_sb(String_Builder* sb, String_View* sv, size_t line){
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

bool sb_getline(String_Builder* sb, FILE *stream){
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

int sb_appendf(String_Builder* sb, const char* fmt, ...){
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

int sb_append_sb(String_Builder* sb_1, String_Builder *sb_2){
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

int sb_append_sv(String_Builder* sb, String_View *sv){
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

void sb_appendc(String_Builder* sb, char c){
    if (sb->count+1 >= sb->capacity){
        size_t expected = sb->capacity == 0? NOTHING_DA_INIT_CAP : sb->capacity;
        da_resize(sb, expected*2);
    }
    
    sb->items[sb->count++] = c;
}

void sb_insertc(String_Builder* sb, char c, size_t pos){
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

int read_entire_file(String_Builder* sb, const char* path){
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
            (*dest)->key = strdup(source->key);
            (*dest)->value = source->value;
            (*dest)->next = source->next;
            
            cur = (KeyValue*)cur->next;
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
            
            free(tmp->key);
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

int hm_put(HashMap* hm, char* key, void* value){
    if (key == NULL) {
        return 1;
    }
    
    unsigned long hash = hash_key((void*)key);
    
    size_t index = hash % hm->capacity;
    
    KeyValue* cur = hm->buckets[index];
    KeyValue* prev = NULL;
    
    while (cur != NULL){
        if (strcmp(cur->key, key) == 0){
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
    
    new->key = strdup(key);
    new->value = value;
    new->next = NULL;
    
    if (prev == NULL){
        hm->buckets[index] = new;
    } else{
        prev->next = (struct KeyValue *)new;
    }
    
    ++hm->count; 
    return 0;
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
    
    unsigned long hash = hash_nkey((void*)key, key_len);
    
    size_t index = hash % hm->capacity;
    
    KeyValue* cur = hm->buckets[index];
    KeyValue* prev = NULL;
    
    while (cur != NULL){
        if (strncmp(cur->key, key, key_len) == 0){
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
    
    new->key = __strndup(key, key_len);
    new->value = value;
    new->next = NULL;
    
    if (prev == NULL){
        hm->buckets[index] = new;
    } else{
        prev->next = (struct KeyValue *)new;
    }
    
    ++hm->count; 
    return 0;
}

void* hm_get(HashMap* hm, const char* key) {
    unsigned long hash = hash_key((void*)key);

    size_t index = hash % hm->capacity;
    KeyValue* cur = hm->buckets[index];
    
    while (cur != NULL){
        if (strcmp(cur->key, key) == 0) return cur->value;
        cur = (KeyValue*)cur->next;
    }
    
    return NULL;
}

void* hm_nget(HashMap* hm, const char* key, size_t key_len) {
    unsigned long hash = hash_nkey((void*)key, key_len);

    size_t index = hash % hm->capacity;
    KeyValue* cur = hm->buckets[index];
    
    while (cur != NULL){
        if (strncmp(cur->key, key, key_len) == 0) return cur->value;
        cur = (KeyValue*)cur->next;
    }
    
    return NULL;
}

unsigned long hash_key(const unsigned char* key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

unsigned long hash_nkey(const unsigned char* key, size_t key_len) {
    unsigned long hash = 5381;
    size_t pos = 0;
    while (pos < key_len) {
        hash = ((hash << 5) + hash) + key[pos];
        ++pos;
    }
    return hash;
}

#endif //NOTHING_IMPLEMENTATION