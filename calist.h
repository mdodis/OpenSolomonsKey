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

#define CA_PUSHBACKDECL(name) CA_PUSHBACKDECL_(name)
#define CA_PUSHBACKDECL_(name) List_##name##_push_back

#define CA_PUSHBACKARRAYDECL(name) CA_PUSHBACKARRAYDECL_(name)
#define CA_PUSHBACKARRAYDECL_(name) List_##name##_push_array

#define ca_push_array(t, name, ptr, cnt) CA_PUSHBACKARRAYDECL(t)(name, ptr, cnt)
#define ca_push_back(t, name, dat) CA_PUSHBACKDECL(t)(name, dat)
#define ca_free(name) do {CA_FREE(name.data); name.data = 0; name.sz = 0; name.cap = 0; } while(0)

#define ca_get(name, i) name.data[i]

#define CA_LISTNAME(name) CA_LISTNAME_(name)
#define CA_LISTNAME_(name) List_##name

#endif /* CALIST_HH */

#if !defined(CA_TYPE)
#error "no CA_TYPE for this implementation!"
#endif

#if !defined(CA_RESERVE_PRESET)
#define CA_RESERVE_PRESET 10
#endif

#define CA_TYPESZ sizeof(CA_TYPE)
#define CA_LIST CA_LISTNAME(CA_TYPE)
#define CA_PUSHBACK CA_PUSHBACKDECL(CA_TYPE)
#define CA_PUSHARRAY CA_PUSHBACKARRAYDECL(CA_TYPE)

typedef struct
{
    CA_TYPE* data;
    unsigned int cap;
    unsigned int sz;
} CA_LIST;

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
        CA_TYPE* new_ptr = (CA_TYPE*) CA_REALLOC(l->data, l->cap * CA_TYPESZ);
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
        puts("Allocate new");
        
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
        puts("(l->sz < l->cap) && (l->cap - l->sz < cnt)");
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

#if defined(CA_RESERVE_PRESET)
#undef CA_RESERVE_PRESET
#endif

#undef CA_LIST
#undef CA_PUSHBACK
#undef CA_PUSHARRAY
#undef CA_TYPESZ
