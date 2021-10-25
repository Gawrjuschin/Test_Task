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

pthread_mutex_t access_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

pthread_rwlock_t rwLock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t spinLock = PTHREAD_SPINLOCK_INITIALIZER;

std::stack<int> stack;
int counter = 0;


void* writer(void*);
void* reader(void*);

void* newReader(void*);
void* newWriter(void*);

int main(int argc, char** argv)
{
  int retval = 0;
  int i = 0;
  pthread_t t_writers[WRITERS_COUNT];
  pthread_t t_readers[READERS_COUNT];

  //Инициализация тредов

  for(i = 0; i < WRITERS_COUNT; ++i)
    {
      if( (retval = pthread_create( &t_writers[i], NULL, &newWriter, NULL)) )
        {
          fprintf(stderr, "Writer thread creation failed: %d\n", retval);
        }
    }
  for(i = 0; i < READERS_COUNT; ++i)
    {
      if( (retval = pthread_create( &t_readers[i], NULL, &newReader, NULL)) )
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


void* writer( void* )
{
  int i = 0;
  int temp = 0;

  // Задержка перед записью, чтобы читатели заснули
  usleep(WRITERS_LATENCY);

  while( 1 )
    {
      pthread_mutex_lock( &access_mutex );

      // Блокировка вывода
      pthread_mutex_lock( &output_mutex );
        printf("Writer#%lld\twrites\n", pthread_self() );
      pthread_mutex_unlock( &output_mutex );

      for( i = 0; i < DATA_SIZE; ++i)
        {
          stack.push(++counter);
        }

      // Актуальное значение счётчика
      temp = counter;

      // Приоритет для писателей: сперва освобождаем мьютекс
      pthread_mutex_unlock( &access_mutex );
      pthread_cond_broadcast( &cond_var );

      // Условие выхода из цикла
      if( temp >= RECORDS_COUNT - DATA_SIZE )
        {
          break;
        }
      // Задержка записи
      usleep(WRITERS_LATENCY);
    }
  pthread_exit(0);
}


void* reader( void* )
{
  int temp = 0;
  int var = 0;

  while( 1 )
    {
      pthread_mutex_lock( &access_mutex );

      while(stack.empty())
        { // POSIX треды могут произвольно просыпаться, поэтому
          // стоит проверять условия выхода. В std::condition_variable
          // передаётся лямбда, которая за это отвечает

          // Блокировка вывода
          pthread_mutex_lock( &output_mutex );
          printf("Thread#%lld\tsleeps\n", pthread_self() );
          pthread_mutex_unlock( &output_mutex );

          pthread_cond_wait( &cond_var, &access_mutex );

          // Блокировка вывода
          pthread_mutex_lock( &output_mutex );
          printf("Thread#%lld\tawakes\n", pthread_self() );
          pthread_mutex_unlock( &output_mutex );
        }

      // Актуальное значения верхнего элемента
      var = stack.top();
      stack.pop();

      // Актуальное значение счётчика
      temp = counter;

      pthread_mutex_unlock( &access_mutex );

      // Блокировка вывода
      pthread_mutex_lock( &output_mutex );
        printf("Thread#%lld\t%d\n", pthread_self(), var );
      pthread_mutex_unlock( &output_mutex );

      // Задержка чтения
      usleep(READERS_LATENCY);

      // Условие выхода из цикла
      if( temp >= RECORDS_COUNT - DATA_SIZE )
        {
          break;
        }

    }
  pthread_exit(0);
}


void* newWriter(void*)
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

      temp = counter;

      // Блокировка вывода
      pthread_spin_lock(&spinLock);
        printf("Writer#%lld\twrites %d\n", pthread_self(), temp);
      pthread_spin_unlock(&spinLock);

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

void* newReader(void*)
{
  int temp = 0;
  int var = 0;
  while (1)
    {
      pthread_rwlock_rdlock(&rwLock);

      var = stack.top();
      temp = counter;

      // Блокировка вывода
      pthread_spin_lock(&spinLock);
        printf("Thread#%lld\treads %d\n", pthread_self(), var );
      pthread_spin_unlock(&spinLock);

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
