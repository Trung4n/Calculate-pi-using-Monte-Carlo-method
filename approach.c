#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

#define MAX_THREADS     1000
#define TOTAL_POINTS    10000000
#define SEED            2112

#define MULTI_FILE      "multi.txt"
#define SHARED_FILE     "shared.txt"
// #define MUTEX 
// #define REDUCE_ATOMIC 
// #define REDUCE_THREAD_LOCAL 
// #define REDUCE_LOCK_FREE 

typedef struct {
    long long points_per_thread;
    long long inside_circle;
    unsigned int seed;
    int shrflg; // flag to decide if using shared variable or not
} ThreadData;

#ifdef MUTEX
pthread_mutex_t lock;
long long global_inside_circle = 0;
#ifdef REDUCE_THREAD_LOCAL
__thread long long thread_local_count = 0; 
#endif
#elif REDUCE_ATOMIC
atomic_long global_inside_circle = 0;
#elif REDUCE_LOCK_FREE
typedef struct {
    long long counter;
} LockFreeCounter;
LockFreeCounter global_inside_circle;
void atomic_add(LockFreeCounter* counter, long long value) {
    long long old_val, new_val;
    do {
        old_val = counter->counter;
        new_val = old_val + value;
    } while (!__sync_bool_compare_and_swap(&counter->counter, old_val, new_val));
}
#endif

void* count_points(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long local_count = 0;
#ifdef REDUCE_THREAD_LOCAL
    const int update_interval = 1000;
#endif
    for (long long i = 0; i < data->points_per_thread; i++) {
        double x = (double)rand_r(&data->seed) / RAND_MAX * 2 - 1;
        double y = (double)rand_r(&data->seed) / RAND_MAX * 2 - 1;
        if (x * x + y * y <= 1.0) {
            local_count++;
        }
#ifdef MUTEX
#ifdef REDUCE_THREAD_LOCAL
    if (data->shrflg && (i == data->points_per_thread - 1 || i % update_interval == 0)) {
        pthread_mutex_lock(&lock);
        global_inside_circle += thread_local_count;
        pthread_mutex_unlock(&lock);
        thread_local_count = 0;
    }
#endif
#endif
    }

    if (data->shrflg) {
#ifdef MUTEX
        pthread_mutex_lock(&lock);
        global_inside_circle += local_count;
        pthread_mutex_unlock(&lock);
#elif REDUCE_ATOMIC
        atomic_fetch_add(&global_inside_circle, local_count);
#elif REDUCE_LOCK_FREE
        atomic_add(&global_inside_circle, local_count);
#endif
    } else {
        data->inside_circle = local_count;
    }

    return NULL;
}

double single_thread(char* argv[]){
    long long inside_circle = 0;
    srand(time(NULL));

    clock_t start_time = clock();
    for (long long i = 0; i < TOTAL_POINTS; i++) {
        double x = (double)rand() / RAND_MAX * 2 - 1;
        double y = (double)rand() / RAND_MAX * 2 - 1;
        if (x * x + y * y <= 1.0) {
            inside_circle++;
        }
    }
    clock_t end_time = clock();

    double pi_estimate = (4.0 * inside_circle) / TOTAL_POINTS;
    double single_thread_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if(strcmp(argv[1], "single") == 0){
        printf("[APPROACH 1] Pi: %.8f | Time: %.4f seconds\n", pi_estimate, single_thread_time);
    }

    return single_thread_time;
}

double multi_thread(double single_thread_time, char* argv[]){
    FILE *fp = NULL;
    int nThreads = atoi(argv[2]);
    int num_threads = nThreads;
    if(strcmp(argv[1], "multi") == 0){
        fp = fopen(MULTI_FILE, "w");
        num_threads = 2;

        if (fp == NULL) {
            printf("Could not open output file.\n");
            return 1;
        }
        
        fprintf(fp, "Threads,EstimatedPi,SpeedUp\n");
    }
    
    double multi_time;
    for (; num_threads <= nThreads; num_threads += 2) {

        pthread_t threads[num_threads];
        ThreadData thread_data[num_threads];

        long long points_per_thread = TOTAL_POINTS / num_threads;
        long long total_inside_circle = 0;

        clock_t start_time = clock();

        for (int i = 0; i < num_threads; i++) {
            thread_data[i].points_per_thread = points_per_thread;
            thread_data[i].inside_circle = 0;
            thread_data[i].shrflg = 0;
            thread_data[i].seed = time(NULL) ^ (i * SEED);
            pthread_create(&threads[i], NULL, count_points, &thread_data[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
            total_inside_circle += thread_data[i].inside_circle;
        }

        clock_t end_time = clock();
  
        multi_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        if(nThreads == num_threads && strcmp(argv[1], "multi") != 0)
            return multi_time;

        double pi_estimate = (4.0 * total_inside_circle) / TOTAL_POINTS;
        double speedup = single_thread_time / multi_time;

        fprintf(fp, "%d,%.8f,%.4f\n", num_threads, pi_estimate, speedup);
        printf("[APPROACH 2][%d threads] Pi: %.8f | Time: %.4f seconds | SpeedUp: %.4f\n", num_threads, pi_estimate, multi_time, speedup);
    }
    fclose(fp);
    int ret = system("python3 plot.py multi");
    if (ret == -1) {
        perror("Failed to run plot.py for multi");
    }
    return ret;
}

void shared_variable(double time_non_shared, char* argv[]) {
    int nThreads = atoi(argv[2]);
    int np = atoi(argv[3]);
    FILE *fp = fopen(SHARED_FILE, "w");
    if (fp == NULL) {
        printf("Could not open output file.\n");
        return;
    }
    fprintf(fp, "nPoints,EstimatedPi,Execution_Time\n");
#ifdef MUTEX
#ifdef REDUCE_THREAD_LOCAL
    printf("Using THREAD LOCAL\n");
#endif
#elif REDUCE_ATOMIC
    printf("Using ATOMIC\n");
#elif REDUCE_LOCK_FREE
    printf("Using LOCK FREE\n");
#endif
    int MAX_POINT = np * TOTAL_POINTS;
    for(int nPoints = TOTAL_POINTS; nPoints <= MAX_POINT; nPoints += TOTAL_POINTS){
        pthread_t threads[nThreads];
        ThreadData thread_data[nThreads];
        long long points_per_thread = nPoints / nThreads;
#ifdef MUTEX
        global_inside_circle = 0;
        pthread_mutex_init(&lock, NULL);
#endif
    
        clock_t start_time = clock();
    
        for (int i = 0; i < nThreads; i++) {
            thread_data[i].points_per_thread = points_per_thread;
            thread_data[i].shrflg = 1;
            thread_data[i].seed = time(NULL) ^ (i * SEED);
            pthread_create(&threads[i], NULL, count_points, &thread_data[i]);
        }
    
        for (int i = 0; i < nThreads; i++) {
            pthread_join(threads[i], NULL);
        }
    
        clock_t end_time = clock();
        double pi_estimate; 
#ifdef MUTEX
        pthread_mutex_destroy(&lock);
        pi_estimate = (4.0 * global_inside_circle) / nPoints;
#elif REDUCE_ATOMIC
        pi_estimate = (4.0 * global_inside_circle) / nPoints;
#elif REDUCE_LOCK_FREE
        pi_estimate = (4.0 * global_inside_circle.counter) / nPoints;
#endif

        double time_shared = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        double speedup = time_non_shared / time_shared;
    
        if(nPoints == TOTAL_POINTS){
            printf("[APPROACH 3] Pi: %.8f | Time: %.4f seconds | SpeedUp: %.4f\n", pi_estimate, time_shared, speedup);
        }
        fprintf(fp, "%d,%.8f,%.4f\n", nPoints, pi_estimate, time_shared);
    }
    fclose(fp);
    int ret = system("python3 plot.py shared");
    if (ret == -1) {
        perror("Failed to run plot.py for shared");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) {
        goto print_usage;
    }

    char* approach = argv[1];
    if (strcmp(approach, "single") == 0 && argc != 2){
        goto print_usage;
    }
    else if (strcmp(approach, "multi") == 0 && argc != 3){
        goto print_usage;
    }
    else if(strcmp(approach, "shared") == 0 && argc != 4){
        goto print_usage;
    }
    else if(strcmp(approach, "single") != 0 && strcmp(approach, "multi") != 0 && strcmp(approach, "shared") != 0){
        goto print_usage;
    }

    double single_time = single_thread(argv);

    if (strcmp(approach, "single") == 0) {
        return 0;
    }

    double multi_time = multi_thread(single_time, argv);

    if (strcmp(approach, "multi") == 0) {
        return 0;
    }

    shared_variable(multi_time, argv);

    if (strcmp(approach, "shared") == 0) {
        return 0;
    }

    print_usage:
        printf("Usage:\n");
        printf("  %s single\n", argv[0]);
        printf("      -> Run Pi estimation with single thread.\n");
        printf("  %s multi <nThreads>\n", argv[0]);
        printf("      -> Run Pi estimation using multiple threads (no shared variable).\n");
        printf("  %s shared <nThreads> <nPointsMultiplier>\n", argv[0]);
        printf("      -> Run Pi estimation with shared variable, scaling number of points.\n");
        return 1;
}