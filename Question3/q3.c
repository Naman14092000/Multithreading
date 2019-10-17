#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#define loop(i, a, b) for (int i = a; i < b; i++)
#define sf(n) scanf("%d", &n);
#define pf(n) printf("%d ", n);
#define pnf(k) printf("%s", k);
#define initial_mutex(a) pthread_mutex_init(a, NULL);
#define create(a, b, c) pthread_create(&a, NULL, (void *)b, &c);
int n, m, k;
int i;
typedef struct taxi
{
    int cab_no;
    int state;
    int rider1;
    int rider2;
    pthread_t thread_ID;
    // sem_t Lock_taxi;
    pthread_mutex_t Lock_taxi;
} cab;
cab *drivers;
typedef struct rider
{
    struct timeval start, end;
    int cab_type;
    int arrival;
    int max_wait;
    int Ride_time;
    int rider_no;
    int state;
    pthread_t thread_ID;
} rider;
int *servers;
int start;
pthread_t *servers_thread_ID;
pthread_mutex_t *Lock_server;
rider *riders;
int rider_comp;
int rides_comp;
int payment_comp;
void make_payment(rider *r);
void book_cab(rider *r)
{
    printf("Rider %d arrived at time %d with %d maxwaittime and %d ridetime and %s cab type\n", r->rider_no, r->arrival - start, r->max_wait, r->Ride_time, r->cab_type == 0 ? "pool" : "premium");
    fflush(stdout);
    while (r->state != 4)
    {

        if (r->cab_type == 0 && r->state == 0)
        {
            loop(i, 1, m + 1)
            {
                pthread_mutex_lock(&drivers[i].Lock_taxi);
                if (drivers[i].state == 1)
                {
                    int tm = time(NULL) - start;
                    printf("Rider %d has started pool ride on cab %d at time %d\n", r->rider_no, i, tm);
                    fflush(stdout);
                    r->state = 1;
                    if (drivers[i].rider2 == -1)
                    {
                        drivers[i].rider2 = r->rider_no;
                    }
                    else if (drivers[i].rider1 == -1)
                    {
                        drivers[i].rider1 = r->rider_no;
                    }
                    drivers[i].state = 2;
                    pthread_mutex_unlock(&drivers[i].Lock_taxi);
                    sleep(r->Ride_time);
                    tm = time(NULL) - start;
                    printf("Rider %d has completed pool ride on cab %d at time %d\n", r->rider_no, i, tm);
                    fflush(stdout);
                    rides_comp++;
                    r->state = 2;
                    break;
                }
                pthread_mutex_unlock(&drivers[i].Lock_taxi);
            }
        }
        if (!r->state)
        {
            loop(i, 1, m + 1)
            {
                pthread_mutex_lock(&drivers[i].Lock_taxi);
                if (drivers[i].state == 0)
                {
                    int tm = time(NULL) - start;
                    printf("Rider %d has started %s ride on cab %d at time %d\n", r->rider_no, r->cab_type == 0 ? "pool" : "premium", i, tm);
                    fflush(stdout);
                    r->state = 1;
                    drivers[i].state = r->cab_type == 0 ? 1 : 3;
                    drivers[i].rider1 = r->rider_no;
                    pthread_mutex_unlock(&drivers[i].Lock_taxi);
                    sleep(r->Ride_time);
                    tm = time(NULL) - start;
                    printf("Rider %d has completed %s ride on cab %d at time %d\n", r->rider_no, r->cab_type == 0 ? "pool" : "premium", i, tm);
                    fflush(stdout);
                    r->state = 2;
                    rides_comp++;
                    break;
                }
                pthread_mutex_unlock(&drivers[i].Lock_taxi);
            }
        }
        if (r->state != 0)
        {
            while (r->state != 3)
                ;
            make_payment(r);
        }
        int ti = time(NULL);
        if ((ti - r->arrival) == r->max_wait && r->state == 0)
        {
            printf("Rider %d returned after waiting\n", r->rider_no);
            fflush(stdout);
            break;
        }
    }
    rider_comp++;
    pthread_exit(NULL);
}
void make_payment(rider *r)
{
    while (r->state != 4)
    {
        loop(i, 1, k + 1)
        {
            pthread_mutex_lock(&Lock_server[i]);
            if (servers[i] == 0)
            {
                servers[i] = 1;
                int tm = time(NULL) - start;
                printf("Rider %d has started payment on server %d at time %d\n", r->rider_no, i, tm);
                fflush(stdout);
                pthread_mutex_unlock(&Lock_server[i]);
                while (servers[i]!=2)
                    ;
                tm = time(NULL) - start;
                printf("Rider %d has made payment on server %d at time %d\n", r->rider_no, i, tm);
                fflush(stdout);
                payment_comp++;
                r->state = 4;
                servers[i]=0;
                break;
            }
            pthread_mutex_unlock(&Lock_server[i]);
        }
    }
}

void end_ride(cab *c, int r)
{
    int tm = time(NULL) - start;
    fflush(stdout);
    if (r == 1)
    {
        riders[c->rider1].state = 3;
        c->rider1 = -1;
    }
    else
    {
        riders[c->rider2].state = 3;
        c->rider2 = -1;
    }
    if (c->state == 3)
    {
        c->state = 0;
        c->rider1 = -1;
        c->rider2 = -1;
    }
    else if (c->state == 1 || c->state == 2)
    {
        c->state -= 1;
    }
    return;
}
void on_ride(cab *c)
{
    while (c->state)
    {
        if (c->rider1 != -1 && riders[c->rider1].state == 2)
        {
            end_ride(c, 1);
        }
        if (c->rider2 != -1 && riders[c->rider2].state == 2)
        {
            end_ride(c, 2);
        }
    }
}
void driver_wait(cab *c)
{
    while (1)
    {
        while (!c->state)
            ;
        on_ride(c);
        if (rider_comp == n)
        {
            break;
        }
    }
    pthread_exit(NULL);
}
void accept_payment(int *s)
{
    while (1)
    {
        while (*s!=1)
            ;
        sleep(2);
        *s = 2;
        if (rider_comp == n)
        {
            break;
        }
    }
    pthread_exit(NULL);
}
int riders_came;
void riders_coming(int rid)
{
    while (riders_came < n)
    {
        int j = rand() % 5;
        sleep(j);
        int k = 1 + rand() % 6;
        while ((n - riders_came) / k == 0)
        {
            k = 1 + rand() % 6;
        }
        int p = 1 + rand() % ((n - riders_came) / k);
        if (p != 0)
        {
            loop(i, riders_came + 1, riders_came + p + 1)
            {
                riders[i].arrival = time(NULL);
                riders[i].max_wait = 10 + rand() % 11;
                riders[i].cab_type = rand() % 2;
                riders[i].Ride_time = 1 + rand() % 10;
                create(riders[i].thread_ID, book_cab, riders[i]);
            }
        }
        riders_came += p;
    }
    pthread_exit(NULL);
}
int main()
{
    riders_came = 0;
    start = time(NULL);
    pnf("Please enter no of Riders : ");
    fflush(stdout);
    sf(n);
    struct timeval t2;
    gettimeofday(&t2, NULL);
    srand(t2.tv_usec * t2.tv_sec);
    riders = (rider *)malloc((n + 1) * sizeof(rider));
    srand(time(NULL));
    loop(i, 1, n + 1)
    {
        riders[i].state = 0;
        riders[i].rider_no = i;
    }
    pnf("Please enter no of cabs : ");
    sf(m);
    drivers = (cab *)malloc((m + 1) * sizeof(cab));
    loop(i, 1, m + 1)
    {
        drivers[i].state = 0; // waitstate
        drivers[i].cab_no = i;
        drivers[i].rider1 = -1;
        drivers[i].rider2 = -1;
        initial_mutex(&drivers[i].Lock_taxi);
        // sem_init(&drivers[i].Lock_taxi, 0, 1);
    }
    pnf("Please enter no of payment servers : ");
    sf(k);
    servers = (int *)malloc((k + 1) * sizeof(int));
    servers_thread_ID = (pthread_t *)malloc((k + 1) * sizeof(pthread_t));
    Lock_server = (pthread_mutex_t *)malloc((k + 1) * sizeof(pthread_mutex_t));
    loop(i, 1, k + 1)
    {
        servers[i] = 0;
        initial_mutex(&Lock_server[i]);
    }
    pthread_t rider_compl;
    create(rider_compl, riders_coming, riders_came);

    loop(i, 1, m + 1)
    {
        create(drivers[i].thread_ID, driver_wait, drivers[i]);
    }
    loop(i, 1, k + 1)
    {
        create(servers_thread_ID[i], accept_payment, servers[i]);
    }
    while (riders_came < n)
        ;
    loop(i, 1, n + 1)
    {
        pthread_join(riders[i].thread_ID, 0);
    }
    free(riders);
    free(drivers);
    free(servers);
    free(servers_thread_ID);
    free(Lock_server);
    // while (rider_comp < n)
    // ;
}