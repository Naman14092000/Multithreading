#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

int *shareMem(size_t size)
{
	key_t mem_key = IPC_PRIVATE;
	int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
	return (int *)shmat(shm_id, NULL, 0);
}

void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
int partition(int a[], int low, int high)
{
	int pivot = low;
	while (low < high)
	{
		while (a[low] <= a[pivot])
		{
			++low;
		}
		while (a[high] > a[pivot])
		{
			--high;
		}
		if (low < high)
		{
			swap(&a[low], &a[high]);
		}
	}
	swap(&a[high], &a[pivot]);
	return high;
}
void sort(int arr[], int low, int high)
{
	if (high <= low)
		return;
	if (high - low + 1 <= 5)
	{
		int a[5], mi = INT_MAX, mid = -1;
		for (int i = low; i < high; i++)
		{
			int j = i + 1;
			for (; j <= high; j++)
				if (arr[j] < arr[i] && j <= high)
				{
					int temp = arr[i];
					arr[i] = arr[j];
					arr[j] = temp;
				}
		}
		return;
	}
	int pivot = partition(arr, low, high);
	sort(arr, low, pivot - 1);
	sort(arr, pivot + 1, high);
}
void quicksort(int *arr, int low, int high)
{
	if (high <= low)
	{
		_exit(1);
	}
	if (high - low + 1 <= 5)
	{
		int a[5], mi = INT_MAX, mid = -1;
		for (int i = low; i < high; i++)
		{
			int j = i + 1;
			for (; j <= high; j++)
				if (arr[j] < arr[i] && j <= high)
				{
					int temp = arr[i];
					arr[i] = arr[j];
					arr[j] = temp;
				}
		}
		return;
	}
	int pid1, pid2;
	int pivot = partition(arr, low, high);
	pid1 = fork();
	if (pid1 == 0)
	{
		quicksort(arr, low, pivot - 1);
		_exit(1);
	}
	else
	{
		pid2 = fork();
		if (pid2 == 0)
		{
			quicksort(arr, pivot + 1, high);
			_exit(1);
		}
		else
		{
			int status;
			waitpid(pid1, &status, 0);
			waitpid(pid2, &status, 0);
		}
	}
	return;
}
struct arg
{
	int l;
	int r;
	int *arr;
};
void *threaded_mergesort(void *a)
{
	//note that we are passing a struct to the threads for simplicity.
	struct arg *args = (struct arg *)a;

	int l = args->l;
	int r = args->r;
	int *arr = args->arr;
	if (l >= r)
		return NULL;
	if (r - l + 1 <= 5)
	{
		int a[5], mi = INT_MAX, mid = -1;
		for (int i = l; i < r; i++)
		{
			int j = i + 1;
			for (; j <= r; j++)
				if (arr[j] < arr[i] && j <= r)
				{
					int temp = arr[i];
					arr[i] = arr[j];
					arr[j] = temp;
				}
		}
		return NULL;
	}
	int pivot = partition(arr, l, r);
	//sort left half array
	struct arg a1;
	a1.l = l;
	a1.r = pivot - 1;
	a1.arr = arr;
	pthread_t tid1;
	pthread_create(&tid1, NULL, threaded_mergesort, &a1);

	//sort right half array
	struct arg a2;
	a2.l = pivot + 1;
	a2.r = r;
	a2.arr = arr;
	pthread_t tid2;
	pthread_create(&tid2, NULL, threaded_mergesort, &a2);

	//wait for the two halves to get sorted
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
}

void runSorts(long long int n)
{

	struct timespec ts;

	//getting shared memory
	int *arr = shareMem(sizeof(int) * (n + 1));
	for (int i = 0; i < n; i++)
		scanf("%d", arr + i);
	int brr[n + 1];
	for (int i = 0; i < n; i++)
		brr[i] = arr[i];
	pthread_t tid;
	struct arg a;
	a.l = 0;
	a.r = n - 1;
	a.arr = brr;
	printf("Running concurrent_quicksort for n = %lld\n", n);
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

	//multiprocess mergesort
	quicksort(arr, 0, n - 1);

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
	printf("time = %Lf\n", en - st);
	long double t1 = en - st;

	printf("Running threaded_concurrent_quicksort for n = %lld\n", n);
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	st = ts.tv_nsec / (1e9) + ts.tv_sec;

	//multithreaded mergesort
	pthread_create(&tid, NULL, threaded_mergesort, &a);
	pthread_join(tid, NULL);

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	en = ts.tv_nsec / (1e9) + ts.tv_sec;
	printf("time = %Lf\n", en - st);
	long double t2 = en - st;

	printf("Running normal_quicksort for n = %lld\n", n);
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	st = ts.tv_nsec / (1e9) + ts.tv_sec;

	// normal mergesort
	sort(brr, 0, n - 1);

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	en = ts.tv_nsec / (1e9) + ts.tv_sec;
	printf("time = %Lf\n", en - st);
	long double t3 = en - st;

	printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n\n", t1 / t3, t2 / t3);

	shmdt(arr);
	return;
}
int main(void)
{
	int n;
	printf("Please print number of inputs : ");
	scanf("%d", &n);
	runSorts(n);
}
