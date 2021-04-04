# htable

A simple, reentrant hash table. Uses FNV-1a to hash keys and linking to resolve collions

## API 

From htable.h ...

```c
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

// Set the key of 'key' to the value of 'val'.
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


// Create a new iterator which acquires a write lock on the table's resources. Work identical to the reading
// iterator, except no other threads may access the htable while the iterator is open.
htable_itr htable_iterator_mut(htable *self);

// Increment the iterator and retrieve the next entry
htable_entry *htable_iterator_next(htable_itr *itr);

// Close the iterator. Calling next after the iterator has been closed will always result in a null pointer
void htable_iterator_destroy(htable_itr *itr);
```
