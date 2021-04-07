#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "htable.h"


int *value_1, *value_2, *value_3, *value_4, *value_5, *value_6, *value_7, *value_8;

int *values[4096];
char *keys[4096];

int reads_large = 0;
int writes_large = 0;

void init_values(char *keys[4096], int *values[4096])
{
  for (int i = 0; i < 4096; i++) {
    int len = (i / 126) + 1;
    char val = (i % 126) + 1;

    // keys are "a", "b", "c" ... "aa", "bb" .. "aaaa", "bbbb", .. so on
    keys[i] = (char *) calloc(len+1, sizeof(int));
    for (int j = 0; j < len; j++) {
      keys[i][j] = val;
    }

    values[i] = (int *) malloc(sizeof(int));
    *values[i] = i;
  }
}

void *write_table_small(void *table)
{
  // Uses iterator to overwrite some values and wait 1s between intervals
  htable_itr itr_mut = htable_iterator_mut((htable *) table);
  htable_entry *entry = NULL;

  while ((entry = htable_iterator_next(&itr_mut)) != NULL) {

    if (strcmp(entry->key, "key 1") == 0) {
      entry->val = value_5;
    } else if (strcmp(entry->key, "key 2") == 0) {
      entry->val = value_6;
    } else if (strcmp(entry->key, "key 3") == 0) {
      entry->val = value_7;
    } else if (strcmp(entry->key, "key 4") == 0) {
      entry->val = value_8;
    }

    sleep(1);
    printf(".\n");
  }

  htable_iterator_destroy(&itr_mut);

  return NULL;
}

void *read_table_small(void *table)
{
  htable_itr itr = htable_iterator((htable *) table);
  htable_entry *entry = NULL;

  while ((entry = htable_iterator_next(&itr)) != NULL) {

    if (strcmp(entry->key, "key 1") == 0) {
      assert(entry->val == value_5);
    } else if (strcmp(entry->key, "key 2") == 0) {
      assert(entry->val == value_6);
    } else if (strcmp(entry->key, "key 3") == 0) {
      assert(entry->val == value_7);
    } else if (strcmp(entry->key, "key 4") == 0) {
      assert(entry->val == value_8);
    }

  }

  htable_iterator_destroy(&itr);

  return NULL;
}

void *write_table_large(void *table)
{

  clock_t start = clock();

  for (clock_t t = clock() - start; t / CLOCKS_PER_SEC < 10; t = clock() - start) {

    int i = rand() % 4096;
    htable_set((htable *) table, keys[i], values[i]);
    writes_large++;

  }

  return NULL;
}

void *read_table_large(void *table)
{

  clock_t start = clock();

  for (clock_t t = clock() - start; t / CLOCKS_PER_SEC < 10; t = clock() - start) {

    int i = rand() % 4096;
    void *val = htable_get((htable *) table, keys[i]);
    reads_large++;
    if (val != NULL) {
      assert(*(int *) val == i);
    }

  }

  return NULL;

}

void *read_write_remove_large(void *table)
{

  clock_t start = clock();

  for (clock_t t = clock() - start; t / CLOCKS_PER_SEC < 10; t = clock() - start) {

    int op = rand() % 2;
    int i = rand() % 4096;
    if (op == 0) {
      htable_set(table, keys[i], values[i]);
      writes_large++;
    } else if (op == 1){
      void *val = htable_get((htable *) table, keys[i]);
      reads_large++;
      if (val != NULL) {
        assert(*(int *) val == i);
      }
    } else {
      assert(*(int *) htable_remove((htable *) table, keys[i]) == *values[i]);
      writes_large++;
    }

  }

  return NULL;
}


int main()
{
  // Test create htable
  htable *tab_small = htable_create(4);
  assert(tab_small != NULL);

  htable *tab_large = htable_create(2048);
  assert(tab_large != NULL);

  printf("htable_create pass\n");

  // Initialize values
  value_1 = malloc(sizeof(int));
  *value_1 = 1;
  value_2 = malloc(sizeof(int));
  *value_2 = 2;
  value_3 = malloc(sizeof(int));
  *value_3 = 3;
  value_4 = malloc(sizeof(int));
  *value_4 = 4;
  value_5 = malloc(sizeof(int));
  *value_5 = 5;
  value_6 = malloc(sizeof(int));
  *value_6 = 6;
  value_7 = malloc(sizeof(int));
  *value_7 = 7;
  value_8 = malloc(sizeof(int));
  *value_8 = 8;

  // Set values
  htable_set(tab_small, "key 1", value_1);
  assert(*(int *) htable_get(tab_small, "key 1") == *value_1);
  assert(htable_size(tab_small) == 1);

  htable_set(tab_small, "key 2", value_2);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_2);
  assert(htable_size(tab_small) == 2);

  htable_set(tab_small, "key 3", value_3);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_3);
  assert(htable_size(tab_small) == 3);

  htable_set(tab_small, "key 4", value_4);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_4);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 5", value_5);
  assert(*(int *) htable_get(tab_small, "key 5") == *value_5);
  assert(htable_size(tab_small) == 5);

  htable_set(tab_small, "key 6", value_6);
  assert(*(int *) htable_get(tab_small, "key 6") == *value_6);
  assert(htable_size(tab_small) == 6);

  htable_set(tab_small, "key 7", value_7);
  assert(*(int *) htable_get(tab_small, "key 7") == *value_7);
  assert(htable_size(tab_small) == 7);

  htable_set(tab_small, "key 8", value_8);
  assert(*(int *) htable_get(tab_small, "key 8") == *value_8);
  assert(htable_size(tab_small) == 8);

  printf("htable_set: pass\n");

  // Get values
  assert(*(int *) htable_get(tab_small, "key 1") == *value_1);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_2);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_3);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_4);
  assert(*(int *) htable_get(tab_small, "key 5") == *value_5);
  assert(*(int *) htable_get(tab_small, "key 6") == *value_6);
  assert(*(int *) htable_get(tab_small, "key 7") == *value_7);
  assert(*(int *) htable_get(tab_small, "key 8") == *value_8);
  assert(htable_get(tab_small, "invalid key") == NULL);
  assert(htable_get(tab_small, "another invalid key") == NULL);

  printf("htable_get: pass\n");

  // Test iterator
  htable_entry *entry = NULL;
  htable_itr itr = htable_iterator(tab_small);

  while ((entry = htable_iterator_next(&itr)) != NULL) {
    // Just test that the entry has some values
    assert(entry->key != NULL);
    assert(entry->val != NULL);
  }

  htable_iterator_destroy(&itr);

  printf("htable_iterator: pass\n");

  // Remove values
  assert(htable_remove(tab_small, "key 1"));
  assert(htable_get(tab_small, "key 1") == NULL);
  assert(htable_size(tab_small) == 7);

  assert(htable_remove(tab_small, "key 2"));
  assert(htable_get(tab_small, "key 2") == NULL);
  assert(htable_size(tab_small) == 6);

  assert(htable_remove(tab_small, "key 3"));
  assert(htable_get(tab_small, "key 3") == NULL);
  assert(htable_size(tab_small) == 5);

  assert(htable_remove(tab_small, "key 4"));
  assert(htable_get(tab_small, "key 4") == NULL);
  assert(htable_size(tab_small) == 4);

  assert(htable_remove(tab_small, "key 5"));
  assert(htable_get(tab_small, "key 5") == NULL);
  assert(htable_size(tab_small) == 3);

  assert(htable_remove(tab_small, "key 6"));
  assert(htable_get(tab_small, "key 6") == NULL);
  assert(htable_size(tab_small) == 2);

  assert(htable_remove(tab_small, "key 7"));
  assert(htable_get(tab_small, "key 7") == NULL);
  assert(htable_size(tab_small) == 1);

  assert(htable_remove(tab_small, "key 8"));
  assert(htable_get(tab_small, "key 8") == NULL);
  assert(htable_size(tab_small) == 0);

  assert(htable_remove(tab_small, "invalid key") == NULL);

  printf("htable_remove: pass\n");

  // Resize table
  htable_set(tab_small, "key 1", value_1);
  assert(*(int *) htable_get(tab_small, "key 1") == *value_1);
  assert(htable_size(tab_small) == 1);

  htable_set(tab_small, "key 2", value_2);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_2);
  assert(htable_size(tab_small) == 2);

  htable_set(tab_small, "key 3", value_3);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_3);
  assert(htable_size(tab_small) == 3);

  htable_set(tab_small, "key 4", value_4);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_4);
  assert(htable_size(tab_small) == 4);

  htable_resize(tab_small, 64);
  assert(tab_small->cap == 64);
  assert(tab_small->size == 4);

  assert(*(int *) htable_get(tab_small, "key 1") == *value_1);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_2);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_3);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_4);
  assert(htable_get(tab_small, "invalid key") == NULL);

  printf("htable_resize: pass\n");

  htable_set(tab_small, "key 1", value_8);
  assert(*(int *) htable_get(tab_small, "key 1") == *value_8);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 2", value_7);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_7);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 3", value_6);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_6);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 4", value_5);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_5);
  assert(htable_size(tab_small) == 4);

  printf("htable_set reset: pass\n");

  // Reset values to match keys
  htable_set(tab_small, "key 1", value_1);
  assert(*(int *) htable_get(tab_small, "key 1") == *value_1);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 2", value_2);
  assert(*(int *) htable_get(tab_small, "key 2") == *value_2);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 3", value_3);
  assert(*(int *) htable_get(tab_small, "key 3") == *value_3);
  assert(htable_size(tab_small) == 4);

  htable_set(tab_small, "key 4", value_4);
  assert(*(int *) htable_get(tab_small, "key 4") == *value_4);
  assert(htable_size(tab_small) == 4);

  pthread_t writer, reader, thread1, thread2;

  printf("testing concurrent read/write...\n");

  pthread_create(&writer, NULL, write_table_small, (void *) tab_small);
  pthread_create(&reader, NULL, read_table_small, (void *) tab_small);

  pthread_join(writer, NULL);
  pthread_join(reader, NULL);

  printf("htable_concurrent read/write: pass\n");

  // Test random read/write. Key & value should always match
  init_values(keys, values);

  printf("testing larger table concurrent read/write...\n");

  pthread_create(&writer, NULL, write_table_large, (void *) tab_large);
  pthread_create(&reader, NULL, read_table_large, (void *) tab_large);
  pthread_create(&thread1, NULL, read_write_remove_large, (void *) tab_large);
  pthread_create(&thread2, NULL, read_write_remove_large, (void *) tab_large);

  pthread_join(writer, NULL);
  pthread_join(reader, NULL);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("larger table concurrency: pass. Reads: ~%i, Writes: ~%i \n", reads_large, writes_large);

  // Test destroy table
  htable_destroy(tab_small);
  htable_destroy(tab_large);
  printf("htable_destroy: pass\n");

  // Free test resources
  free(value_1);
  free(value_2);
  free(value_3);
  free(value_4);
  free(value_5);
  free(value_6);
  free(value_7);
  free(value_8);

  return 0;
}
