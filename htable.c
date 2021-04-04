#include "htable.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

// helpers

// FNV-1a hash algorithm, taken from Ben Hoyt's C hash table implementation
// https://benhoyt.com/writings/hash-table-in-c/
static uint64_t hash_fn(const char *key)
{
  uint64_t hash = FNV_OFFSET;
  for (const char *p = key; *p; p++) {
    hash ^= (uint64_t) (unsigned char) (*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

static htable_node *htable_node_create(const char *key, void *val)
{
  htable_node *self;

  if ((self = malloc(sizeof(htable_node))) == NULL) {
    return NULL;
  }

  self->entry.key = key;
  self->entry.val = val;
  self->next = NULL;

  return self;
}

static void htable_node_destroy(htable_node *self)
{
  free(self);
}

static htable_node *htable_iterator_next_node(htable_itr *itr)
{
  // Get the next element in the table. If there are no more elements, then null is returned
  htable_node *node = NULL;

  // Get entry linked to the current
  if (itr->node != NULL && itr->node->next != NULL) {

    node = itr->node->next;
    itr->node = itr->node->next;

  } else {

    // Get entry from the next bucket
    while (node == NULL && itr->next_bucket < itr->tab->cap) {

      if (itr->tab->buckets[itr->next_bucket] != NULL) {
        node = itr->tab->buckets[itr->next_bucket];
      }

      itr->next_bucket++;

    }

    // If there are no linked entries and no more buckets, the iterator is finished
  }

  return node;
}


// htable

htable *htable_create(size_t size)
{
  htable *self;

  if ((self = calloc(1, sizeof(htable))) == NULL) {
    return NULL;
  }

  if ((self->buckets = calloc(size, sizeof(htable_node *))) == NULL) {
    free(self);
    return NULL;
  }

  self->size = 0;
  self->cap = size;

  pthread_rwlock_init(&self->mu, NULL);

  return self;
}

void htable_destroy(htable *self)
{
  // Free all elements of the table
  htable_itr itr = htable_iterator_mut(self);
  htable_node *node = NULL;

  while ((node = htable_iterator_next_node(&itr)) != NULL) {
    free(node);
  }

  htable_iterator_destroy(&itr);

  pthread_rwlock_destroy(&self->mu);
  free(self->buckets);
  free(self);
}

int htable_size(htable *self)
{
  return self->size;
}

void htable_set(htable *self, const char *key, void *val)
{
  uint64_t hash = hash_fn(key);
  size_t bucket = (size_t) (hash & self->cap - 1);

  pthread_rwlock_wrlock(&self->mu);

  htable_node *node = self->buckets[bucket];

  if (node == NULL) {

    // Create new entry in bucket
    self->buckets[bucket] = htable_node_create(key, val);;
    self->size++;

  } else {

    bool is_set = false;

    // Iterate through the list and find a matching entry
    for (; node->next; node = node->next) {
      if (strcmp(node->entry.key, key) == 0) {
        node->entry.val = val;
        is_set = true;
        break;
      }
    }

    // If no entry was found, create a new one at the end of the list
    if (!is_set) {
      if (strcmp(node->entry.key, key) == 0) {
        node->entry.val = val;
      } else {
        node->next = htable_node_create(key, val);;
        self->size++;
      }
    }

  }

  pthread_rwlock_unlock(&self->mu);
}

void *htable_get(htable *self, const char *key)
{
  uint64_t hash = hash_fn(key);
  size_t bucket = (size_t) (hash & self->cap - 1);

  pthread_rwlock_rdlock(&self->mu);

  htable_node *node = self->buckets[bucket];

  // Check each element in the bucket for a key match. The loop will terminate
  // when the reach the end or we find an entry with a matching key
  for (; node != NULL && strcmp(node->entry.key, key) != 0; node = node->next);

  pthread_rwlock_unlock(&self->mu);
  return node == NULL ? NULL : node->entry.val;
}

void htable_remove(htable *self, const char *key)
{
  uint64_t hash = hash_fn(key);
  size_t bucket = (size_t) (hash & self->cap - 1);

  pthread_rwlock_wrlock(&self->mu);

  htable_node *node = self->buckets[bucket];

  if (node != NULL) {

    if (strcmp(node->entry.key, key) == 0) {

      htable_node *next = node->next;
      htable_node_destroy(node);
      self->size--;

      // If there is another element in the bucket, move it up
      if (next != NULL) {
        self->buckets[bucket] = next;
      } else {
        self->buckets[bucket] = NULL;
      }

    } else {

      // Find matching entry and connect previous entry to it
      node = node->next;
      htable_node *last = node;

      while (node != NULL && strcmp(node->entry.key, key) != 0) {

        last = node;
        node = node->next;

      }

      // If entry is not null, reconnect last to next and free the entry
      if (node != NULL) {
        last->next = node->next;
        htable_node_destroy(node);
        self->size--;
      }

    }

  }

  pthread_rwlock_unlock(&self->mu);
}

void htable_resize(htable *self, size_t size)
{
  htable *resized = htable_create(size);

  htable_itr itr = htable_iterator_mut(self);

  // Get iterator without locking mutex
  htable_node *node = NULL;

  // Iterate all entries
  while ((node = htable_iterator_next_node(&itr)) != NULL) {
    htable_set(resized, node->entry.key, node->entry.val);

    // Free old node node
    free(node);
  }

  // Free old bucket array
  free(self->buckets);

  // Reassign resized to self
  self->buckets = resized->buckets;
  self->size = resized->size;
  self->cap = resized->cap;

  // Free temporary table
  free(resized);

  htable_iterator_destroy(&itr);
}


// htable_itr

htable_itr htable_iterator(htable *self)
{
  // Lock the table for reading. htable_iterator_close must be called to unlock the mutex
  pthread_rwlock_rdlock(&self->mu);

  return (htable_itr) {
          NULL,
          self,
          0
  };
}

htable_itr htable_iterator_mut(htable *self)
{
  // Lock the table for writing. htable_iterator_close must be called to unlock the mutex
  pthread_rwlock_wrlock(&self->mu);

  return (htable_itr) {
          NULL,
          self,
          0
  };
}

htable_entry *htable_iterator_next(htable_itr *itr)
{
  // Get the next element in the table. If there are no more elements, then null is returned
  htable_node *node = NULL;

  // Get entry linked to the current
  if (itr->node != NULL && itr->node->next != NULL) {

    node = itr->node->next;
    itr->node = itr->node->next;

  } else {

    // Get entry from the next bucket
    while (node == NULL && itr->next_bucket < itr->tab->cap) {

      if (itr->tab->buckets[itr->next_bucket] != NULL) {
        node = itr->tab->buckets[itr->next_bucket];
      }

      itr->next_bucket++;

    }

    // If there are no linked entries and no more buckets, the iterator is finished
  }

  return node == NULL ? NULL : &node->entry;
}

void htable_iterator_destroy(htable_itr *itr)
{
  pthread_rwlock_unlock(&itr->tab->mu);
}
