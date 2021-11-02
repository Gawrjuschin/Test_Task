#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

//Использую плюсовый стек для удобства
#include <stack>

#define WRITERS_COUNT 2
#define READERS_COUNT 3
#define WRITERS_LATENCY 100000
#define READERS_LATENCY 100000
#define RECORDS_COUNT 100
#define DATA_SIZE 5

pthread_rwlock_t rwLock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t outputLock = PTHREAD_SPINLOCK_INITIALIZER;

std::stack<int> stack;
int counter = 0;

void* reader(void*);
void* writer(void*);

int main(int argc, char** argv)
{
  int retval = 0;
  int i = 0;
  pthread_t t_writers[WRITERS_COUNT];
  pthread_t t_readers[READERS_COUNT];

  //Инициализация тредов

  for(i = 0; i < WRITERS_COUNT; ++i)
    {
      if( (retval = pthread_create( &t_writers[i], NULL, &writer, NULL)) )
        {
          fprintf(stderr, "Writer thread creation failed: %d\n", retval);
        }
    }
  for(i = 0; i < READERS_COUNT; ++i)
    {
      if( (retval = pthread_create( &t_readers[i], NULL, &reader, NULL)) )
        {
          fprintf(stderr, "Reader thread creation failed: %d\n", retval);
        }
    }

  // Ожидание завершения

  for(i = 0; i < WRITERS_COUNT; ++i)
    {
      pthread_join( t_writers[i], NULL);
    }
  for(i = 0; i < READERS_COUNT; ++i)
    {
      pthread_join( t_readers[i], NULL);
    }

  fprintf(stderr, "Done!\n");
  system("pause");

  return 0;
}

void* writer(void*)
{
  int temp = 0;
  int i = 0;
  while (1)
    {
      pthread_rwlock_wrlock(&rwLock);

      for( i = 0; i < DATA_SIZE; ++i)
        {
          stack.push(++counter);
        }

      // Актуальное значение счётчика
      temp = counter;

      // Блокировка вывода
      pthread_spin_lock(&outputLock);
        printf("Writer#%lld\twrites %d\n", pthread_self(), temp);
      pthread_spin_unlock(&outputLock);

      pthread_rwlock_unlock(&rwLock);

      // Условие выхода из цикла
      if(temp >= RECORDS_COUNT - DATA_SIZE)
        {
          break;
        }
      // Задержка записи
      usleep(WRITERS_LATENCY);
    }
  pthread_exit(0);
}

void* reader(void*)
{
  int temp = 0;
  int var = 0;
  while (1)
    {
      pthread_rwlock_rdlock(&rwLock);

      // Актуальные значения данных и счётчика
      var = stack.top();
      temp = counter;

      // Блокировка вывода
      pthread_spin_lock(&outputLock);
        printf("Thread#%lld\treads %d\n", pthread_self(), var );
      pthread_spin_unlock(&outputLock);

      pthread_rwlock_unlock(&rwLock);

      // Условие выхода из цикла
      if( temp >= RECORDS_COUNT - DATA_SIZE )
        {
          break;
        }
      // Задержка чтения
      usleep(READERS_LATENCY);
    }

  pthread_exit(0);
}
