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

static htable_node *htable_node_create(htable *self, const char *key, void *val)
{
  htable_node *node;

  if ((node = self->alloc(1, sizeof(htable_node))) == NULL) {
    return NULL;
  }

  node->entry.key = key;
  node->entry.val = val;
  node->next = NULL;

  return node;
}

static void htable_node_destroy(htable *self, htable_node *node)
{
  self->dealloc(node);
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
  return htable_create_with_allocator(calloc, free, size);
}

htable *htable_create_with_allocator(void *(*alloc)(size_t, size_t), void (*dealloc)(void *), size_t size)
{
  htable *self;

  if ((self = alloc(1, sizeof(htable))) == NULL) {
    return NULL;
  }

  if ((self->buckets = alloc(size, sizeof(htable_node *))) == NULL) {
    dealloc(self);
    return NULL;
  }

  self->alloc = alloc;
  self->dealloc = dealloc;
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
    htable_node_destroy(self, node);
  }

  htable_iterator_destroy(&itr);

  pthread_rwlock_destroy(&self->mu);
  self->dealloc(self->buckets);
  self->dealloc(self);
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
    self->buckets[bucket] = htable_node_create(self, key, val);;
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
        node->next = htable_node_create(self, key, val);;
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

void *htable_remove(htable *self, const char *key)
{
  uint64_t hash = hash_fn(key);
  size_t bucket = (size_t) (hash & self->cap - 1);
  void *value = NULL;

  pthread_rwlock_wrlock(&self->mu);

  htable_node *node = self->buckets[bucket];

  if (node != NULL) {

    if (strcmp(node->entry.key, key) == 0) {

      htable_node *next = node->next;
      value = node->entry.val;
      htable_node_destroy(self, node);
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
        value = node->entry.val;
        htable_node_destroy(self, node);
        self->size--;
      }

    }

  }

  pthread_rwlock_unlock(&self->mu);
  return value;
}

void htable_resize(htable *self, size_t size)
{
  htable *resized = htable_create_with_allocator(self->alloc, self->dealloc, size);
  htable_itr itr = htable_iterator_mut(self);
  htable_node *node = NULL;

  // Iterate all entries
  while ((node = htable_iterator_next_node(&itr)) != NULL) {
    htable_set(resized, node->entry.key, node->entry.val);

    // Free old node
    htable_node_destroy(self, node);
  }

  // Free old bucket array
  self->dealloc(self->buckets);

  // Reassign resized to self
  self->buckets = resized->buckets;
  self->size = resized->size;
  self->cap = resized->cap;

  // Free temporary table
  self->dealloc(resized);

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
