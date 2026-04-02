#ifndef NOTHING_H
#define NOTHING_H

#include <stddef.h>
#include <string.h>
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

#define da_remove_unordered(da, i)                   \
    do {                                             \
        size_t j = (i);                              \
        (da)->items[j] = (da)->items[--(da)->count]; \
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

int read_entire_file(String_Builder* sb, const char* path);
void sv_trim(String_View* sv);
void sv_trim_left(String_View* sv);
void sv_trim_right(String_View* sv);

int sb_get_line(String_Builder* sb, String_View* sv, size_t line);
int sb_appendf(String_Builder* sb, const char* fmt, ...);
void sb_appendc(String_Builder* sb, char c);

HashMap* hm_alloc();
void hm_reset(HashMap* hm);
void hm_free(HashMap* hm);
int hm_put(HashMap* hm, char* key, void* value);
void* hm_get(HashMap* hm, const char* key);
unsigned long hash_key(const unsigned char* key);

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

int sb_to_sv(String_Builder* sb, String_View* sv, size_t start, size_t count){
    if (start+count >= sb->count+1) return 0;
    
    sv->count = count;
    sv->items = sb->items+start;
    return 1;
}

void sv_trim(String_View* sv){
    sv_trim_left(sv);
    sv_trim_right(sv);
}

void sv_trim_left(String_View* sv){
    while (sv->count > 0 && iswblank(sv->items[0])){
        sv->items = ++sv->items;
        --sv->count;
    }
}

void sv_trim_right(String_View* sv){
    while (sv->count > 0 && iswblank(sv->items[sv->count - 1])){
        --sv->count;
    }
}

char* sv_to_cstr(String_View* sv){
    char* cstr = NOTHING_MALLOC(sizeof(char)*(sv->count+1));
    for (size_t i = 0; i < sv->count; ++i){
        cstr[i] = sv->items[i];
    }
    
    cstr[sv->count] = 0;
    return cstr;
}

int sb_get_line(String_Builder* sb, String_View* sv, size_t line){
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

void sb_appendc(String_Builder* sb, char c){
    if (sb->count+1 >= sb->capacity){
        size_t expected = sb->capacity == 0? NOTHING_DA_INIT_CAP : sb->capacity;
        da_resize(sb, expected*2);
    }
    
    sb->items[sb->count++] = c;
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

HashMap* hm_alloc(){
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
    if (key == NULL) return 1;
    
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

unsigned long hash_key(const unsigned char* key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

#endif //NOTHING_IMPLEMENTATION