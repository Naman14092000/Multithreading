#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#define loop(i, a, b) for (int i = a; i < b; i++)
#define sf(n) scanf("%d", &n);
#define pf(n) printf("%d ", n);
#define pnf(k) printf("%s", k);
#define initial_mutex(a) pthread_mutex_init(a, NULL);
#define create(a, b, c) pthread_create(&a, NULL, (void *)b, &c);
int i;
int n, m, k;
int yet_to_be_cooked;
int students_rem;
pthread_mutex_t student_gaya;
typedef struct robot
{
    int robot_no;
    int state;
    int no_of_vessel;
    int seconds;
    int stud_served;
    pthread_t thread_ID;
} robot;
robot *robots;
typedef struct container
{
    int container_num;
    int slots;
    pthread_mutex_t Lock_vessel;
    int served;
    int capacity;
    int state;
    int wait;
    pthread_t thread_ID;

} vessel;
vessel *vessels;
typedef struct student
{
    int state;
    int student_num;
    int vessel_no;
    pthread_t thread_ID;

} stud;
stud *students;
int students_served = 0;
void student_food(stud *s)
{
    int p = s->vessel_no;
    printf("Student %d is getting food at table %d\n", s->student_num, p);
    fflush(stdout);
    s->state = 1;
    pthread_mutex_lock(&vessels[p].Lock_vessel);
    vessels[p].served--;
    vessels[p].slots--;
    vessels[p].capacity--;
    pthread_mutex_unlock(&vessels[p].Lock_vessel);
}
void student_wait(stud *s)
{
    printf("Student %d arrived\n", s->student_num);
    fflush(stdout);
    while (s->state != 1)
    {
        if (s->state == 0)
        {
            loop(i, 1, m + 1)
            {

                pthread_mutex_lock(&vessels[i].Lock_vessel);
                if (vessels[i].state == 1 && vessels[i].slots > 0 && vessels[i].served != vessels[i].slots)
                {

                    printf("Student %d has been alloted a slot on container %d\n", s->student_num, vessels[i].container_num);
                    fflush(stdout);
                    s->state = 2;
                    s->vessel_no = i;
                    vessels[i].served++;
                    students_served++;
                    pthread_mutex_unlock(&vessels[i].Lock_vessel);
                    break;
                }
                pthread_mutex_unlock(&vessels[i].Lock_vessel);
            }
        }
        if (s->state == 2)
        {
            while (vessels[s->vessel_no].served != vessels[s->vessel_no].slots)
            {
                if (students_served == k)
                {
                    break;
                }
            }
            student_food(s);
        }
    }
    pthread_mutex_lock(&student_gaya);
    students_rem--;
    pthread_mutex_unlock(&student_gaya);
    pthread_exit(NULL);
}
void ready_to_serve(vessel *v)
{
    while (1)
    {
        if (students_rem == 0)
        {
            return;
        }
        int slots = v->slots;
        v->state = 1;
        if (slots == 0)
        {
            v->state = 2;
            printf("container %d with capacity=%d has finished all slots\n", v->container_num, v->capacity);
            fflush(stdout);
            return;
        }
    }
}
void vessel_wait(vessel *v)
{
    struct timeval t2;
    gettimeofday(&t2, NULL);
    srand(t2.tv_usec * t2.tv_sec);
    while (1)
    {
        while (!v->state)
        {
        }
        if (v->capacity > 0)
        {
            v->slots = 1 + rand() % v->capacity;
            printf("container %d with capacity=%d and available slots=%d is ready to serve\n", v->container_num, v->capacity, v->slots);
            fflush(stdout);
            v->served = 0;
            ready_to_serve(v);
        }
        if (students_rem == 0)
        {
            break;
        }
        if (v->capacity == 0)
        {
            v->state = 0;
            printf("container %d is waiting to be filled\n", v->container_num);
            fflush(stdout);
        }
    }
    pthread_exit(NULL);
}
void biryani_ready(robot *r)
{
    int number = r->no_of_vessel;
    while (number)
    {
        if (students_rem == 0)
        {
            pthread_mutex_unlock(&vessels[i].Lock_vessel);
            return;
        }
        loop(i, 1, m + 1)
        {
            pthread_mutex_lock(&vessels[i].Lock_vessel);
            if (vessels[i].capacity <= 0 && !vessels[i].state)
            {
                number--;
                printf("Chef %d has filled food in container %d\n", r->robot_no, i);
                fflush(stdout);
                vessels[i].state = 2; // filled
                vessels[i].capacity = r->stud_served;
            }
            if (!number)
            {
                printf("Chef %d again started cooking\n", r->robot_no);
                fflush(stdout);
                pthread_mutex_unlock(&vessels[i].Lock_vessel);
                return;
            }
            if (students_rem == 0)
            {
                pthread_mutex_unlock(&vessels[i].Lock_vessel);
                return;
            }
            pthread_mutex_unlock(&vessels[i].Lock_vessel);
        }
    }
}
void preparing_biryani(robot *r)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec * t1.tv_sec);
    while (1)
    {
        if (students_rem == 0)
        {
            break;
        }
        r->state = 1; // preparing
        r->no_of_vessel = 1 + rand() % 10;
        r->seconds = 2 + rand() % 4;
        r->stud_served = 25 + rand() % 26;
        int j = r->seconds;
        sleep(j);
        if (students_rem < r->stud_served)
        {
            r->stud_served = 1 + rand() % students_rem;
            r->no_of_vessel = 1;
        }
        if (students_rem == 0)
        {
            break;
        }
        r->state = 2;
        printf("Chef %d has prepared %d vessels of biryani with %d capacity each\n", r->robot_no, r->no_of_vessel, r->stud_served);
        fflush(stdout);
        biryani_ready(r);
    }
    pthread_exit(NULL);
}
int students_came = 0;
void students_coming(int rid)
{
    while (students_came < k)
    {
        int j = rand() % 4;
        sleep(j);
        int p = 1 + rand() % 6;
        while ((k - students_came) / p == 0)
        {
            p = 1 + rand() % 6;
        }
        int q = 1 + rand() % ((k - students_came) / p);
        if (q != 0)
        {
            loop(i, students_came + 1, students_came + q + 1)
            {
                create(students[i].thread_ID, student_wait, students[i]);
            }
        }
        students_came += q;
    }
    pthread_exit(NULL);
}
int main()
{
    pnf("number of robotchef : ");
    fflush(stdout);
    sf(n);
    robots = (robot *)malloc((n + 1) * sizeof(robot));
    // srand(time(NULL));
    loop(i, 1, n + 1)
    {
        robots[i].robot_no = i;
        robots[i].state = 0; // doing nothing
        fflush(stdout);
    }
    pnf("number of vessel : ");
    fflush(stdout);

    sf(m);
    vessels = (vessel *)malloc((m + 1) * sizeof(vessel));
    loop(i, 1, m + 1)
    {
        vessels[i].container_num = i;
        vessels[i].capacity = 0;
        vessels[i].state = 0; // empty
        vessels[i].slots = 0;
        vessels[i].wait = 1;
        vessels[i].served = 0;
        initial_mutex(&vessels[i].Lock_vessel);
    }
    pnf("number of students : ");
    fflush(stdout);

    sf(k);
    students = (stud *)malloc((k + 1) * sizeof(stud));
    students_rem = k;
    yet_to_be_cooked = k;
    loop(i, 1, k + 1)
    {
        students[i].state = 0; //waiting
        students[i].student_num = i;
        // initial_mutex(&students[i].Lock_student);
    }
    pnf("Mess started!!\n");
    fflush(stdout);
    loop(i, 1, n + 1)
    {
        create(robots[i].thread_ID, preparing_biryani, robots[i]);
    }
    loop(i, 1, m + 1)
    {
        create(vessels[i].thread_ID, vessel_wait, vessels[i]);
    }
    // loop(i, 1, k + 1)
    // {
    //     create(students[i].thread_ID, student_wait, students[i]);
    // }
    initial_mutex(&student_gaya);
    pthread_t students_cam;
    create(students_cam, students_coming, students_came);
    while (students_came < k)
        ;
    loop(i, 1, k + 1)
    {
        pthread_join(students[i].thread_ID, 0);
    }
    pnf("Mess is done for today!\n");
    fflush(stdout);
    free(vessels);
    free(robots);
    free(students);
    return 0;
}