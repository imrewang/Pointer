#include <stdio.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/******************cleanup.h*********************/

#define on_destroy(T, _cleanup)  __attribute__((cleanup(_cleanup))) T


/*-----------------------cleanup.h-----------------------*/
/******************sp_count.h***********************/

/* called when shared ptr expired */
typedef void (*sp_deleter_fn_t)(void* );

typedef struct sp_counter {
    int cnt_share;
    int cnt_weak;

    void* resourse;
    sp_deleter_fn_t deleter;
} sp_counter_t;


/*-----------------sp_count.h-----------------------*/
/******************sp_count.c***********************/

sp_counter_t* sp_counter_new()
{
    sp_counter_t* c = (sp_counter_t* )malloc(sizeof(sp_counter_t));

    memset(c, 0, sizeof(sp_counter_t));
    return c;
}

void sp_counter_init(sp_counter_t* c)
{
    c->cnt_share = c->cnt_weak = 1;
    c->resourse = NULL;
    c->deleter = free; /* default is free */
}

void sp_counter_delete(sp_counter_t* c)
{
    free(c);
}

void sp_counter_incweak(sp_counter_t* c)
{
    __sync_fetch_and_add(&c->cnt_weak, 1);
}

void sp_counter_decweak(sp_counter_t* c)
{
    int weak = __sync_add_and_fetch(&c->cnt_weak, -1);
    if (weak == 0){
        sp_counter_delete(c);
    }
}

void sp_counter_incshare(sp_counter_t* c)
{
    __sync_fetch_and_add(&c->cnt_share, 1);
}

void sp_counter_decshare(sp_counter_t* c)
{

    if (__sync_add_and_fetch(&c->cnt_share, -1) == 0) {
        if (c->resourse && c->deleter) {

            c->deleter(c->resourse);
        }

        sp_counter_decweak(c);
    }
}

int sp_counter_try_incshare(sp_counter_t* c)
{
    /*
     * When a shared_ptr is construct from weakptr
     * The weak ptr may be expired, so here is a infinite loop
     * to check whether the weak ptr is valid
     */
    while(1) {
        const int oldCnt = c->cnt_share;
        if (oldCnt == 0)
            return 0;

        if (oldCnt == __sync_val_compare_and_swap(&c->cnt_share, oldCnt, oldCnt + 1))
            return 1;
    }

    return 0;
}

/*-----------------sp_count.c-----------------------*/
/******************sp_shared_ptr.h***********************/
struct sp_counter;
struct sp_weak_ptr;

typedef void (*sp_deleter_fn_t)(void* );

typedef struct sp_shared_ptr {
    struct sp_counter* counter;
} sp_shared_ptr_t;

sp_shared_ptr_t* shared_ptr_new(void* res);//#define

sp_shared_ptr_t* shared_ptr_new_with_deleter(void* res, sp_deleter_fn_t del);//#define

sp_shared_ptr_t* shared_ptr_copy(sp_shared_ptr_t* p);//#define copy

sp_shared_ptr_t* shared_ptr_from_weak(struct sp_weak_ptr * wk);//#define lock

void* shared_ptr_get(sp_shared_ptr_t* p);//get

int shared_ptr_use_count(sp_shared_ptr_t* p);//use_count

void shared_ptr_swap(sp_shared_ptr_t* p1,sp_shared_ptr_t* p2);//swap

#define MAKE_SHARED_RESET(var, T) shared_ptr_reset(var,malloc(sizeof(T)))

void shared_ptr_reset(sp_shared_ptr_t* p,void* res);//reset

/* for gcc cleanup */
void shared_ptr_destructor(sp_shared_ptr_t** p);

#define MAKE_SHARED(var, T) on_destroy(sp_shared_ptr_t*, shared_ptr_destructor) var = shared_ptr_new(malloc(sizeof(T)))

#define MAKE_SHARED_DELETER(var, T, del) on_destroy(sp_shared_ptr_t*, shared_ptr_destructor) var = shared_ptr_new_with_deleter(malloc(sizeof(T)), del)

#define MAKE_SHARED_COPY(to, from) on_destroy(sp_shared_ptr_t*, shared_ptr_destructor) to = shared_ptr_copy(from)

#define WEAK_PTR_LOCK(var, weak) on_destroy(sp_shared_ptr_t*, shared_ptr_destructor) var = weak_ptr_lock(weak)

/*-----------------sp_shared_ptr.h-----------------------*/
/******************sp_weak_ptr.h***********************/
struct sp_shared_ptr;
struct sp_counter;

typedef struct sp_weak_ptr {
    struct sp_counter* counter;
} sp_weak_ptr_t;

sp_weak_ptr_t* weak_ptr_new();

//void weak_ptr_delete(sp_weak_ptr_t* wp);

sp_weak_ptr_t* weak_ptr_copy(sp_weak_ptr_t* wp) ;

sp_weak_ptr_t* weak_ptr_from_shared(struct sp_shared_ptr* sp) ;

struct sp_shared_ptr* weak_ptr_lock(sp_weak_ptr_t* wp);//lock

int weak_ptr_expired(sp_weak_ptr_t* wp) ;//expired

int weak_ptr_use_count(sp_weak_ptr_t* wp);//use_count

void weak_ptr_swap(sp_weak_ptr_t* wp1,sp_weak_ptr_t* wp2);//swap

/* for gcc cleanup */
void weak_ptr_destructor(sp_weak_ptr_t** p);

#define MAKE_WEAKPTR(var, shared_ptr) on_destroy(sp_weak_ptr_t*, weak_ptr_destructor) var = weak_ptr_from_shared(shared_ptr)
#define MAKE_WEAKPTR_COPY(to, from) on_destroy(sp_weak_ptr_t*, weak_ptr_destructor) to = weak_ptr_copy(from)
/*-----------------sp_weak_ptr.h-----------------------*/
/******************sp_shared_ptr.c***********************/

static
void shared_ptr_init(sp_shared_ptr_t* p)
{
    p->counter = NULL;
}

static
void shared_ptr_init_with_resourse(sp_shared_ptr_t* p, void* res)
{
    if (!res) {
        shared_ptr_init(p);
    } else {
        p->counter = sp_counter_new();
        sp_counter_init(p->counter);
        p->counter->resourse = res;
    }
}

sp_shared_ptr_t* shared_ptr_new(void* res)
{
    sp_shared_ptr_t* p = (sp_shared_ptr_t* )malloc(sizeof(sp_shared_ptr_t));
    if (res){
        shared_ptr_init_with_resourse(p, res);
    }
    else{
        shared_ptr_init(p);
    }
    return p;
}


sp_shared_ptr_t* shared_ptr_new_with_deleter(void* res, sp_deleter_fn_t del)
{
    sp_shared_ptr_t* p = (sp_shared_ptr_t* )malloc(sizeof(sp_shared_ptr_t));
    if (res) {
        shared_ptr_init_with_resourse(p, res);
        p->counter->deleter = del;
    } else {
        shared_ptr_init(p);
    }

    return p;
}

sp_shared_ptr_t* shared_ptr_copy(sp_shared_ptr_t* p)
{
    if (!p) {
        return NULL;
    }

    sp_shared_ptr_t* p2 = shared_ptr_new(NULL);
    p2->counter = p->counter;
    if (p2->counter){
        sp_counter_incshare(p2->counter);
    }

    return p2;
}

sp_shared_ptr_t* shared_ptr_from_weak(sp_weak_ptr_t* wk)
{
    if (!wk) {
        return NULL;
    }

    sp_counter_t* tmp = wk->counter;
    if (tmp && sp_counter_try_incshare(tmp)) {
        sp_shared_ptr_t* sp = shared_ptr_new(NULL);
        sp->counter = tmp;
        return sp;
    } else {
        return NULL;
    }
}

void* shared_ptr_get(sp_shared_ptr_t* p)
{

    return p->counter->resourse;
}


int shared_ptr_use_count(sp_shared_ptr_t* p){
    if(!p||!p->counter){
        return -1;
    }
    return p->counter->cnt_share;
}


void shared_ptr_swap(sp_shared_ptr_t* p1,sp_shared_ptr_t* p2){
    if(!p1||!p2){
        return ;
    }

    sp_shared_ptr_t* temp_ptr = shared_ptr_new(NULL);
    temp_ptr->counter = p1->counter;
    p1->counter=p2->counter;
    p2->counter=temp_ptr->counter;
    free(temp_ptr);
}


void shared_ptr_reset(sp_shared_ptr_t* p,void* res){
    if(!p){
        return ;
    }

    if (p->counter){
        sp_counter_decshare(p->counter);
    }

    p->counter = sp_counter_new();
    sp_counter_init(p->counter);
    p->counter->resourse = res;
}

void shared_ptr_destructor(sp_shared_ptr_t** pp)
{
    if (!pp || !*pp){
        return;
    }

    sp_shared_ptr_t* p = *pp;

    if (p && p->counter){
        sp_counter_decshare(p->counter);
    }

    printf("~~ free shared_ptr: %p\n", p);

    free(p);
    *pp = NULL;
}

/*-----------------sp_shared_ptr.c-----------------------*/
/******************sp_weak_ptr.c***********************/

static
void weak_ptr_init(sp_weak_ptr_t* wp)
{
    wp->counter = NULL;
}

sp_weak_ptr_t* weak_ptr_new()
{
    sp_weak_ptr_t* wp = (sp_weak_ptr_t* )malloc(sizeof(sp_weak_ptr_t));
    weak_ptr_init(wp);

    return wp;
}

sp_weak_ptr_t* weak_ptr_copy(sp_weak_ptr_t* wp)
{
    if (!wp) {
        return NULL;
    }

    sp_weak_ptr_t* p = weak_ptr_new();
    if (wp->counter) {
        p->counter = wp->counter;
        sp_counter_incweak(p->counter);
    } else {
        weak_ptr_init(p);
    }

    return p;
}

sp_weak_ptr_t* weak_ptr_from_shared(sp_shared_ptr_t* sp)
{
    if (!sp) {
        return NULL;
    }

    sp_weak_ptr_t* wp = weak_ptr_new();
    if (sp->counter) {
        wp->counter = sp->counter;
        sp_counter_incweak(sp->counter);
    } else {
        weak_ptr_init(wp);
    }

    return wp;
}

sp_shared_ptr_t* weak_ptr_lock(sp_weak_ptr_t* wp)
{
    if (!wp) {
        return NULL;
    }

    return shared_ptr_from_weak(wp);
}

int weak_ptr_expired(sp_weak_ptr_t* wp)
{
    if (!wp || !wp->counter) {
        return 1;
    }

    int share_cnt = __sync_fetch_and_add(&wp->counter->cnt_share, 0);
    return share_cnt == 0 ? 1 : 0;
}

int weak_ptr_use_count(sp_weak_ptr_t* wp){
    if(!wp||!wp->counter){
        return -1;
    }
    return wp->counter->cnt_share;
}

void weak_ptr_swap(sp_weak_ptr_t* wp1,sp_weak_ptr_t* wp2){
    if(!wp1||!wp2){
        return ;
    }

    sp_weak_ptr_t* temp_ptr = weak_ptr_new(NULL);
    temp_ptr->counter = wp1->counter;
    wp1->counter=wp2->counter;
    wp2->counter=temp_ptr->counter;
    free(temp_ptr);
}

void weak_ptr_destructor(sp_weak_ptr_t** wpp)
{
    if (!wpp || !*wpp){
        return;
    }

    sp_weak_ptr_t* wp = *wpp;
    if (wp && wp->counter){
        sp_counter_decweak(wp->counter);
    }

    printf("~~ free weak_ptr: %p\n", wp);

    free(wp);
}
/*-----------------sp_weak_ptr.c-----------------------*/
#define ptr_use_count(A)\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_shared_ptr_t*),\
              shared_ptr_use_count(*(sp_shared_ptr_t**)&A),\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_weak_ptr_t*),\
              weak_ptr_use_count(*(sp_weak_ptr_t**)&A),\
    printf("don't support this type parameter,please check again\n")))



#define ptr_swap(A,B)\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_shared_ptr_t*),\
              shared_ptr_swap(*(sp_shared_ptr_t**)&A,*(sp_shared_ptr_t**)&B),\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_weak_ptr_t*),\
              weak_ptr_swap(*(sp_weak_ptr_t**)&A,*(sp_weak_ptr_t**)&B),\
    printf("don't support this type parameter,please check again\n")))

typedef struct Data {
    char a[32];
} Data_t;


void test_raii(){
    printf("***********test_raii***********\n\n");
    Data_t* d1;
    {
        MAKE_SHARED(p1, Data_t);//MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));

        Data_t* d1 = (Data_t*)shared_ptr_get(p1);//Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        printf("Set p1 data : aaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        strncpy(d1->a, "aaaaaaaaaaaaaaaaaaaaaaaaaa", 27);
        printf("p1 data: %s\n",d1->a);
    }
    if(d1==NULL){
        printf("OK: d1==NULL");
    }
    else{
        printf("ERROR: d1!=NULL");
    }
    printf("\n***********test_raii pass***********\n");
}

void test_shared_ptr() {
    printf("***********test_shared_ptr***********\n\n");
    {
        MAKE_SHARED(p1, Data_t);//MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));

        MAKE_SHARED(p2, Data_t);//MAKE_SHARED(p2, Data_t);
        printf("Initialize the shared pointer p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));
        printf("-- p2->counter->cnt_share:%d\n", ptr_use_count(p2));

        MAKE_SHARED_COPY(p3, p2);//MAKE_SHARED_COPY(p3, p2);
        printf("Initialize p3 with p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));
        printf("-- p2->counter->cnt_share:%d\n", ptr_use_count(p2));
        printf("-- p3->counter->cnt_share:%d\n", ptr_use_count(p3));

        ptr_swap(p1,p2);//ptr_ptr_swap(p1,p2);
        printf("\nExchange shared pointers p1, p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));
        printf("-- p2->counter->cnt_share:%d\n", ptr_use_count(p2));
        printf("-- p3->counter->cnt_share:%d\n", ptr_use_count(p3));

        Data_t* d1 = (Data_t*)shared_ptr_get(p1);//Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        Data_t* d3 = (Data_t*)shared_ptr_get(p3);//Data_t* d3 = (Data_t*)shared_ptr_get(p3);
        printf("Set p1 data : aaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        strncpy(d1->a, "aaaaaaaaaaaaaaaaaaaaaaaaaa", 27);

        printf("p3 data: %s\n", d3->a);

        if (d1 == d3) {
            printf("OK: p1==p3\n");
        }
        else {
            printf("ERROR: p1!=p3\n");
        }

        printf("\nActively destroy the shared pointer p2:\n");
        shared_ptr_destructor(&p2);//shared_ptr_destructor(&p2);

        printf("\nReset p3\n");
        MAKE_SHARED_RESET(p3,Data_t);//MAKE_SHARED_RESET(p3,Data_t);
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));
        printf("-- p3->counter->cnt_share:%d\n", ptr_use_count(p3));

        Data_t* d33 = (Data_t*)shared_ptr_get(p3);//Data_t* d33  = (Data_t*)shared_ptr_get(p3);
        strncpy(d33->a, "bbbbbbbbbbbbbbbbbbbbbbbbbb", 27);
        printf("Set p3 data : bbbbbbbbbbbbbbbbbbbbbbbbbb\n");
        printf("p3 data: %s\n", d33->a);
        printf("p1 data: %s\n\n",d1->a);
    }
    printf("\n***********test_shared_ptr pass***********\n");

}


void test_weak_ptr() {
    printf("\n\n***********test_weak_ptr***********\n\n");

    {

        MAKE_SHARED(p1, Data_t);//MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d,p1->counter->cnt_weak:%d\n", ptr_use_count(p1), p1->counter->cnt_weak - 1);

        Data_t* d1 = (Data_t*)shared_ptr_get(p1);//Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        printf("Set p1 data:ccccccccccccccccccc\n");
        strncpy(d1->a, "ccccccccccccccccccc", 20);

        MAKE_SHARED(p2, Data_t);//MAKE_SHARED(p2, Data_t);
        printf("Initialize the shared pointer p2:\n");
        printf("-- p2->counter->cnt_share:%d,p2->counter->cnt_weak:%d\n",ptr_use_count(p2), p2->counter->cnt_weak - 1);

        Data_t* d2 = (Data_t*)shared_ptr_get(p2);//Data_t* d2 = (Data_t*)shared_ptr_get(p2);
        printf("Set p2 data:dddddddddddd\n");
        strncpy(d2->a, "dddddddddddd", 13);

        MAKE_WEAKPTR(w1, p1);//MAKE_WEAKPTR(w1, p1);
        printf("\nInitialize weak_ptr w1 with p1\n");
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", ptr_use_count(w1), w1->counter->cnt_weak - 1);
        printf("-- p1->counter->cnt_share:%d,p1->counter->cnt_weak:%d\n", ptr_use_count(w1), p1->counter->cnt_weak - 1);

        MAKE_WEAKPTR(w2, p2);//MAKE_WEAKPTR(w2, p2);
        printf("Initialize weak_ptr w2 with p2\n");
        printf("-- w2->counter->cnt_share:%d,w2->counter->cnt_weak:%d\n", ptr_use_count(w2), w2->counter->cnt_weak - 1);
        printf("-- p2->counter->cnt_share:%d,p2->counter->cnt_weak:%d\n", ptr_use_count(p2), p2->counter->cnt_weak - 1);

        WEAK_PTR_LOCK(sw, w1);//WEAK_PTR_LOCK(sw, w1);
        printf("\nConvert w1 to a shared pointer sw:\n");
        printf("-- sw->counter->cnt_share:%d,sw->counter->cnt_weak:%d\n",ptr_use_count(sw), sw->counter->cnt_weak - 1);
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", ptr_use_count(w1), w1->counter->cnt_weak - 1);

        Data_t* swd = (Data_t*)shared_ptr_get(sw); //Data_t* swd = (Data_t*)shared_ptr_get(sw);
        printf("sw data: %s\n", swd->a);

        ptr_swap(w1,w2);//ptr_ptr_swap(w1,w2);
        printf("\nExchange weak pointers w1, w2:\n");
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", ptr_use_count(w1), w1->counter->cnt_weak - 1);
        printf("-- w2->counter->cnt_share:%d,w2->counter->cnt_weak:%d\n", ptr_use_count(w2), w2->counter->cnt_weak - 1);


        printf("\nActively destroy the shared pointer p2:\n");
        shared_ptr_destructor(&p2);//shared_ptr_destructor(&p2);
        if (weak_ptr_expired(w1)) {//if(weak_ptr_expired(w1)){
            printf("OK: Now weak_ptr w1 expires\n");
        }
        else {
            printf("ERROR: Now weak_ptr w1 has not expired\n");
        }

        if (weak_ptr_expired(w2)) {//if(weak_ptr_expired(w2)){
            printf("ERROR: Now weak_ptr w2 expires\n\n");
        }
        else {
            printf("OK: Now weak_ptr w2 has not expired\n\n");
        }
    }
    printf("\n***********test_weak_ptr pass***********\n");

}




int main() {
    test_raii();

    test_shared_ptr();

    test_weak_ptr();

    printf("\nALL PASS\n");
    return 0;
}




