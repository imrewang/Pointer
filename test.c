#include <stdio.h>
#include <assert.h>

typedef struct Data
{
    char a[32];
} Data_t;

void test_raii()
{
    printf("***********test_raii***********\n\n");
    Data_t *d1;
    {
        std::shared_ptr<Data_t> p1 = std::make_shared<Data_t>(); // MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d\n", ptr_use_count(p1));

        d1 = p1.get(); // Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        printf("Set p1 data : aaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        strncpy(d1->a, "aaaaaaaaaaaaaaaaaaaaaaaaaa", 27);
        printf("p1 data: %s\n", d1->a);
    }
    if (d1 == NULL)
    {
        printf("OK: d1==NULL");
    }
    else
    {
        printf("ERROR: d1!=NULL");
    }
    printf("\n***********test_raii pass***********\n");
}

void test_shared_ptr()
{
    printf("***********test_shared_ptr***********\n\n");
    {
        std::shared_ptr<Data_t> p1 = std::make_shared<Data_t>(); // MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d\n", p1.use_count());

        std::shared_ptr<Data_t> p2 = std::make_shared<Data_t>(); // MAKE_SHARED(p2, Data_t);
        printf("Initialize the shared pointer p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", p1.use_count());
        printf("-- p2->counter->cnt_share:%d\n", p2.use_count());

        std::shared_ptr<Data_t> p3 = p2; // MAKE_SHARED_COPY(p3, p2);
        printf("Initialize p3 with p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", p1.use_count());
        printf("-- p2->counter->cnt_share:%d\n", p2.use_count());
        printf("-- p3->counter->cnt_share:%d\n", p3.use_count());

        swap(p1, p2); // ptr_swap(p1,p2);
        printf("\nExchange shared pointers p1, p2:\n");
        printf("-- p1->counter->cnt_share:%d\n", p1.use_count());
        printf("-- p2->counter->cnt_share:%d\n", p2.use_count());
        printf("-- p3->counter->cnt_share:%d\n", p3.use_count());

        Data_t *d1 = p1.get(); // Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        Data_t *d3 = p3.get(); // Data_t* d3 = (Data_t*)shared_ptr_get(p3);
        printf("Set p1 data : aaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        strncpy(d1->a, "aaaaaaaaaaaaaaaaaaaaaaaaaa", 27);

        printf("p3 data: %s\n", d3->a);

        if (d1 == d3)
        {
            printf("OK: p1==p3\n");
        }
        else
        {
            printf("ERROR: p1!=p3\n");
        }

        printf("\nActively destroy the shared pointer p2:\n");
        p2.~shared_ptr(); // shared_ptr_destructor(&p2);

        printf("\nReset p3\n");
        p3.reset(new Data_t); // MAKE_SHARED_RESET(p3,Data_t);
        printf("-- p1->counter->cnt_share:%d\n", p1.use_count());
        printf("-- p3->counter->cnt_share:%d\n", p3.use_count());

        Data_t *d33 = p3.get(); // Data_t* d33  = (Data_t*)shared_ptr_get(p3);
        strncpy(d33->a, "bbbbbbbbbbbbbbbbbbbbbbbbbb", 27);
        printf("Set p3 data : bbbbbbbbbbbbbbbbbbbbbbbbbb\n");
        printf("p3 data: %s\n", d33->a);
        printf("p1 data: %s\n\n", d1->a);
    }
    printf("\n***********test_shared_ptr pass***********\n");
}

void test_weak_ptr()
{
    printf("\n\n***********test_weak_ptr***********\n\n");

    {

        std::shared_ptr<Data_t> p1 = std::make_shared<Data_t>(); // MAKE_SHARED(p1, Data_t);
        printf("Initialize the shared pointer p1:\n");
        printf("-- p1->counter->cnt_share:%d,p1->counter->cnt_weak:%d\n", p1.use_count(), p1->counter->cnt_weak - 1);

        Data_t *d1 = p1.get(); // Data_t* d1 = (Data_t*)shared_ptr_get(p1);
        printf("Set p1 data:ccccccccccccccccccc\n");
        strncpy(d1->a, "ccccccccccccccccccc", 20);

        std::shared_ptr<Data_t> p2 = std::make_shared<Data_t>(); // MAKE_SHARED(p2, Data_t);
        printf("Initialize the shared pointer p2:\n");
        printf("-- p2->counter->cnt_share:%d,p2->counter->cnt_weak:%d\n", p2.use_count(), p2->counter->cnt_weak - 1);

        Data_t *d2 = p2.get(); // Data_t* d2 = (Data_t*)shared_ptr_get(p2);
        printf("Set p2 data:dddddddddddd\n");
        strncpy(d2->a, "dddddddddddd", 13);

        std::weak_ptr<Data_t> w1(p1); // MAKE_WEAKPTR(w1, p1);
        printf("\nInitialize weak_ptr w1 with p1\n");
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", w1.use_count(), w1->counter->cnt_weak - 1);
        printf("-- p1->counter->cnt_share:%d,p1->counter->cnt_weak:%d\n", w1.use_count(), p1->counter->cnt_weak - 1);

        std::weak_ptr<Data_t> w2(p2); // MAKE_WEAKPTR(w2, p2);
        printf("Initialize weak_ptr w2 with p2\n");
        printf("-- w2->counter->cnt_share:%d,w2->counter->cnt_weak:%d\n", w2.use_count(), w2->counter->cnt_weak - 1);
        printf("-- p2->counter->cnt_share:%d,p2->counter->cnt_weak:%d\n", p2.use_count(), p2->counter->cnt_weak - 1);

        std::shared_ptr<Data_t> sw = w1.lock(); // WEAK_PTR_LOCK(sw, w1);
        printf("\nConvert w1 to a shared pointer sw:\n");
        printf("-- sw->counter->cnt_share:%d,sw->counter->cnt_weak:%d\n", sw.use_count(), sw->counter->cnt_weak - 1);
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", w1.use_count(), w1->counter->cnt_weak - 1);

        Data_t *swd = sw.get(); // Data_t* swd = (Data_t*)shared_ptr_get(sw);
        printf("sw data: %s\n", swd->a);

        swap(w1, w2); // ptr_swap(w1,w2);
        printf("\nExchange weak pointers w1, w2:\n");
        printf("-- w1->counter->cnt_share:%d,w1->counter->cnt_weak:%d\n", w1.use_count(), w1->counter->cnt_weak - 1);
        printf("-- w2->counter->cnt_share:%d,w2->counter->cnt_weak:%d\n", w2.use_count(), w2->counter->cnt_weak - 1);

        printf("\nActively destroy the shared pointer p2:\n");
        p2.~shared_ptr(); // shared_ptr_destructor(&p2);
        if (w1.expired())
        { // if(weak_ptr_expired(w1)){
            printf("OK: Now weak_ptr w1 expires\n");
        }
        else
        {
            printf("ERROR: Now weak_ptr w1 has not expired\n");
        }

        if (w2.expired())
        { // if(weak_ptr_expired(w2)){
            printf("ERROR: Now weak_ptr w2 expires\n\n");
        }
        else
        {
            printf("OK: Now weak_ptr w2 has not expired\n\n");
        }
    }
    printf("\n***********test_weak_ptr pass***********\n");
}

int main()
{
    test_raii();

    test_shared_ptr();

    test_weak_ptr();

    printf("\nALL PASS\n");
    return 0;
}
