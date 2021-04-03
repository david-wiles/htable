#include <assert.h>
#include <stdio.h>

#include "htable.h"


int main()
{
  // Test create htable
  htable *tab_small = htable_create(4);
  assert(tab_small != NULL);

  htable *tab_large = htable_create(2048);
  assert(tab_large != NULL);

  printf("htable_create pass\n");

  // Initialize values
  int *value_1 = malloc(sizeof(int));
  *value_1 = 1;
  int *value_2 = malloc(sizeof(int));
  *value_2 = 2;
  int *value_3 = malloc(sizeof(int));
  *value_3 = 3;
  int *value_4 = malloc(sizeof(int));
  *value_4 = 4;
  int *value_5 = malloc(sizeof(int));
  *value_5 = 5;
  int *value_6 = malloc(sizeof(int));
  *value_6 = 6;
  int *value_7 = malloc(sizeof(int));
  *value_7 = 7;
  int *value_8 = malloc(sizeof(int));
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
  htable_remove(tab_small, "key 1");
  assert(htable_get(tab_small, "key 1") == NULL);
  assert(htable_size(tab_small) == 7);

  htable_remove(tab_small, "key 2");
  assert(htable_get(tab_small, "key 2") == NULL);
  assert(htable_size(tab_small) == 6);

  htable_remove(tab_small, "key 3");
  assert(htable_get(tab_small, "key 3") == NULL);
  assert(htable_size(tab_small) == 5);

  htable_remove(tab_small, "key 4");
  assert(htable_get(tab_small, "key 4") == NULL);
  assert(htable_size(tab_small) == 4);

  htable_remove(tab_small, "key 5");
  assert(htable_get(tab_small, "key 5") == NULL);
  assert(htable_size(tab_small) == 3);

  htable_remove(tab_small, "key 6");
  assert(htable_get(tab_small, "key 6") == NULL);
  assert(htable_size(tab_small) == 2);

  htable_remove(tab_small, "key 7");
  assert(htable_get(tab_small, "key 7") == NULL);
  assert(htable_size(tab_small) == 1);

  htable_remove(tab_small, "key 8");
  assert(htable_get(tab_small, "key 8") == NULL);
  assert(htable_size(tab_small) == 0);

  htable_remove(tab_small, "invalid key");

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

  // TODO test threaded operations

  // Test destroy table
  htable_destroy(tab_small);
  htable_destroy(tab_large);
  printf("htable_destroy: pass\n");

  return 0;
}
