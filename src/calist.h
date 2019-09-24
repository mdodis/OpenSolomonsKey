/* calist.h
Michael Dodis, 2019
Portable vector like array list implementation

USAGE:
 - Define CA_TYPE to be the name of the type in the list
 - Inlclude the header file
 - undefine CA_TYPE
 - Do it all over again for each type
 
 CUSTOMIZATION:
 CA_RESERVE_PRESET | Initial reserve amount of CA_TYPE on push_back
 
#define CA_TYPE int
#include "calist.h"
#undef CA_TYPE

List_int my_list = {};
ca_push_back(int, &my_list, 100);
ca_free(my_list);

*/
#if !defined(CALIST_HH)
#define CALIST_HH

#if !defined(CA_MALLOC)
#define CA_MALLOC malloc
#endif
#if !defined(CA_REALLOC)
#define CA_REALLOC realloc
#endif
#if !defined(CA_FREE)
#define CA_FREE free
#endif

/* Push Back*/
#define CA_PUSHBACKDECL(name) CA_PUSHBACKDECL_(name)
#define CA_PUSHBACKDECL_(name) List_##name##_push_back
/* Push Whole array*/
#define CA_PUSHBACKARRAYDECL(name) CA_PUSHBACKARRAYDECL_(name)
#define CA_PUSHBACKARRAYDECL_(name) List_##name##_push_array
/* Delete index*/
#define CA_DELETEIDXDECL(name) CA_DELETEIDXDECL_(name)
#define CA_DELETEIDXDECL_(name) List_##name##_delete_idx
/* Reserve */
#define CA_RESERVEDECL(name) CA_RESERVEDECL_(name)
#define CA_RESERVEDECL_(name) List_##name##_reserve
/* Pop front */
#define CA_POPFRONTDECL(name) CA_POPFRONTDECL_(name)
#define CA_POPFRONTDECL_(name) List_##name##_pop_front
/* Front */
#define CA_FRONTDECL(name) CA_FRONTDECL_(name)
#define CA_FRONTDECL_(name) List_##name##_front

#define ca_reserve(t, name, cnt) CA_RESERVEDECL(t)(name, cnt)

/* Short-hand macros*/
#define ca_push_array(t, name, ptr, cnt) CA_PUSHBACKARRAYDECL(t)(name, ptr, cnt)
#define ca_push_back(t, name, dat) CA_PUSHBACKDECL(t)(name, dat)
#define ca_free(name) do {CA_FREE(name.data); name.data = 0; name.sz = 0; name.cap = 0; } while(0)

#define ca_get(name, i) name.data[i]
#define ca_front(t, name) CA_FRONTDECL(t)(name)

#define ca_delete_idx(t, name, i) CA_DELETEIDXDECL(t)(name, i)
#define ca_pop_front(t, name) CA_POPFRONT(t)(name)

#define CA_LISTNAME(name) CA_LISTNAME_(name)
#define CA_LISTNAME_(name) List_##name

#endif /* CALIST_HH */

#if !defined(CA_TYPE)
#error "no CA_TYPE for this implementation!"
#endif

#if !defined(CA_RESERVE_PRESET)
#define CA_RESERVE_PRESET 10
#endif

#define CA_TYPESZ    sizeof(CA_TYPE)
#define CA_LIST      CA_LISTNAME(CA_TYPE)
#define CA_PUSHBACK  CA_PUSHBACKDECL(CA_TYPE)
#define CA_PUSHARRAY CA_PUSHBACKARRAYDECL(CA_TYPE)
#define CA_DELETEIDX CA_DELETEIDXDECL(CA_TYPE)
#define CA_RESERVE   CA_RESERVEDECL(CA_TYPE)
#define CA_POPFRONT  CA_POPFRONTDECL(CA_TYPE)
#define CA_FRONT     CA_FRONTDECL(CA_TYPE)

typedef struct
{
    CA_TYPE* data;
    unsigned int cap;
    unsigned int sz;
} CA_LIST;

char CA_RESERVE(CA_LIST* const l, unsigned int cnt)
{
    if (l->sz) return false;
    l->data = (CA_TYPE*) CA_MALLOC(cnt * CA_TYPESZ);
    if (!l->data) return false;
    l->cap = cnt;
    return true;
}

char CA_PUSHBACK(CA_LIST* const l, CA_TYPE t)
{
    if (l->cap == 0)
    {
        /* Reserve x spots*/
        l->data = (CA_TYPE*) CA_MALLOC(CA_RESERVE_PRESET * CA_TYPESZ);
        if (!l->data) return 0;
        
        l->cap = CA_RESERVE_PRESET;
        l->sz = 0;
    }
    
    if (l->sz == l->cap)
    {
        /* Resize */
        l->cap *= 2;
        CA_TYPE* new_ptr = (CA_TYPE*)CA_REALLOC(l->data, l->cap * CA_TYPESZ);
        if (!new_ptr) return 0;
        l->data = new_ptr;
    }
    
    l->data[l->sz] = t;
    l->sz++;
    return 1;
}

char CA_PUSHARRAY(CA_LIST* const l, CA_TYPE* dat, u32 cnt)
{
    if (l->cap == 0)
    {
        l->data = (CA_TYPE*) CA_MALLOC(cnt * CA_TYPESZ);
        if (!l->data) return 0;
        // puts("Allocate new");
        
        l->cap = cnt;
        l->sz = 0;
    }
    else if ((l->sz < l->cap) && (l->cap - l->sz < cnt))
    {
        /* Resize */
        l->cap = cnt - (l->cap - l->sz < cnt);
        CA_TYPE* new_ptr = (CA_TYPE*)CA_REALLOC(l->data, l->cap * CA_TYPESZ);
        if (!new_ptr) return 0;
        l->data = new_ptr;
        // puts("(l->sz < l->cap) && (l->cap - l->sz < cnt)");
    }
    else if (l->sz == l->cap)
    {
        /* Resize */
        l->cap += cnt;
        CA_TYPE* new_ptr = (CA_TYPE*)CA_REALLOC(l->data, l->cap * CA_TYPESZ);
        if (!new_ptr) return 0;
        l->data = new_ptr;
    }
    
    int off = l->sz;
    for (int i = 0; i < cnt; ++i)
    {
        l->data[off + i] = dat[i];
    }
    
    l->sz += cnt;
    return 1;
}

void CA_DELETEIDX(CA_LIST* const l, unsigned int index)
{
    long int idx = index;
    if (index >= l->sz) return;
    
    if (idx == (long int)(l->sz) - 1)
    {
        /* Top item */
        //l->data[idx] = {};
        if (l->sz > 0)
            l->sz--;
        return;    
    }
    
    //l->data[idx] = {};
    idx++;
    for (; idx < l->sz; ++idx)
    {
        l->data[idx - 1] = l->data[idx];
    }
    
    l->sz--;
    
    return;
}

CA_TYPE* CA_FRONT(CA_LIST* const l)
{
    if (l->sz <= 0) return 0;
    else return l->data + (l->sz - 1);
}

void CA_POPFRONT(CA_LIST* const l)
{
    if (l->sz <= 0) return;
    l->sz--;
}

#if defined(CA_RESERVE_PRESET)
#undef CA_RESERVE_PRESET
#endif

#undef CA_LIST
#undef CA_PUSHBACK
#undef CA_PUSHARRAY
#undef CA_TYPESZ