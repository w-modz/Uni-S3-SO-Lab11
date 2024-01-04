// SO IS1 221A LAB11
// Wiktor Modzelewski
// mw53766@zut.edu.pl
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <processthreadsapi.h>

#define MAX_SIZE 1000000
#define ALLOWED_ARGUMENT_COUNT 3

float* table;
int table_lenght, thread_count;
float global_sum = 0.0;
// Struktura dla watkow aby przekazac argumenty do funkcji
typedef struct {
    float* array;
    int size;
    HANDLE mutex;
} ThreadData;

// Funkcja dla watkow
DWORD sumThread(void* thread_arguments) {
    ThreadData* data = (ThreadData*)thread_arguments;

    printf("Thread ID:%d size=%d\n", GetCurrentThreadId(), data->size);
    float partial_sum = 0.0;
    for (int i = 0; i < data->size; i++) {
        partial_sum += data->array[i];
    }

    WaitForSingleObject(data->mutex, INFINITE);
    global_sum += partial_sum;
    ReleaseMutex(data->mutex);

    printf("Thread ID:%d sum=%.6f\n", GetCurrentThreadId(), partial_sum);

    return 0;
}

int main(int argc, char* argv[]) {
    // Weryfikowanie argumentow
    if (argc != ALLOWED_ARGUMENT_COUNT) {
        printf("Invalid number of arguments.\n");
        return EXIT_FAILURE;
    }
    table_lenght = atoi(argv[1]);
    thread_count = atoi(argv[2]);
    if (table_lenght <= 0 || table_lenght >= MAX_SIZE || thread_count <= 0 || thread_count >= table_lenght) {
        printf("Invalid argument.\n");
        return EXIT_FAILURE;
    }
    table = (float*)malloc(table_lenght * sizeof(float));
    
    srand(0);
    for (int i = 0; i < table_lenght; ++i) {
        table[i] = 1000. * rand() / RAND_MAX;
    }

    HANDLE* threads = (HANDLE*)malloc(thread_count * sizeof(HANDLE));
    ThreadData* thread_data = (ThreadData*)malloc(thread_count * sizeof(ThreadData));
    HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    int elements_per_thread = table_lenght / thread_count;
    int extra_elements = table_lenght % thread_count;
    int index = 0;

    clock_t start_time, end_time;
    start_time = clock();
    float total_time;

    // Tworzenie watkow
    for (int i = 0; i < thread_count; i++) {
        thread_data[i].array = &table[i * elements_per_thread];
        thread_data[i].size = (i < thread_count - 1 || extra_elements == 0) ? elements_per_thread : elements_per_thread + extra_elements;
        thread_data[i].mutex = mutex;

        threads[i] = CreateThread(NULL, 0, sumThread, &thread_data[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Error during thread creation.\n");
            return EXIT_FAILURE;
        }
    }

    WaitForMultipleObjects(thread_count, threads, TRUE, INFINITE);
    end_time = clock();
    total_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("w/Threads: sum=%f, time=%f seconds\n", global_sum, total_time);

    // Sumowanie bez watkow
    global_sum = 0.0;
    start_time = clock();
    for (int i = 0; i < table_lenght; ++i) {
        global_sum += table[i];
    }
    end_time = clock();
    total_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("wo/Threads: sum=%f, time=%f seconds\n", global_sum, total_time);

    free(table);
    CloseHandle(mutex);
    free(threads);
    free(thread_data);

    return 0;
}