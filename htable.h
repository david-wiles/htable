#ifndef HTABLE_HTABLE_H
#define HTABLE_HTABLE_H


#include <stdlib.h>
#include <pthread.h>


#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL


typedef struct htable_entry
{
  const char *key;
  void *val;
} htable_entry;

typedef struct htable_node
{
  htable_entry entry;
  struct htable_node *next;
} htable_node;

typedef struct htable
{
  size_t size; // Number of entries currently stored in the table

  htable_node **buckets; // Main array. Each bucket is a linked list
  size_t cap;            // Capacity of the array
  pthread_rwlock_t mu;   // read/write mutex
} htable;

typedef struct htable_itr
{
  htable_node *node; // The current entry
  htable *tab;         // Reference to the original table, used to lock and unlock read mutex
  size_t next_bucket;  // Current bucket the iterator is pointing at
} htable_itr;


// htable

// Create a new htable which can contain the specified number of elements.
//
// Note that the hash table can contain more than size elements through the use of linked
// collision resolution, but the underlying table itself will not be resized automatically.
// Resizing must be done using htable_resize(), passing in the desired new size of the table
htable *htable_create(size_t size);

// Destroy the htable and free all resources
// This will also attempt to free all values stored in the table
void htable_destroy(htable *self);

// Get the number of elements currently in the table
int htable_size(htable *self);

// Set the key of 'key' to the value of 'entry'.
void htable_set(htable *self, const char *key, void *val);

// Get the value stored with 'key'. If the key does not exist, then a null pointer is returned.
void *htable_get(htable *self, const char *key);

// Remove the value stored with 'key', if it exists. If the value does exist, then it will be freed.
void htable_remove(htable *self, const char *key);

// Resizes the table to the specified size
//
// This will not remove elements, only change the size of the underlying array. Choosing a size smaller
// than the number of elements currently in the table will not fail, but will result in a greater number
// of elements will collisions, so the size of the array should be used when resizing the array unless
// other constraints are placed on the table
void htable_resize(htable *self, size_t size);


// htable_itr

// Create a new iterator from the table. htable_iterator_next() is used to increment the iterator and
// retrieve the next element
//
// While the iterator is open, the table will retain a read lock on the elements to preserve consistency.
// Therefore, the iterator should be closed once it is no longer in use.
htable_itr htable_iterator(htable *self);

// Increment the iterator and retrieve the next entry
htable_entry *htable_iterator_next(htable_itr *itr);

// Close the iterator. Calling next after the iterator has been closed will always result in a null pointer
void htable_iterator_destroy(htable_itr *itr);

#endif //HTABLE_HTABLE_H
