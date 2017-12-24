#include "task-creation.h"

slice_t * task_creation( double * v, int n, int * task_n ) {
  slice_t * tasks;
  int count = 0;
  int last_slice = 0;
  int i;

  tasks = (slice_t *)malloc(sizeof(slice_t)*n/TASK_TRIGGER);
  *task_n = 0;

  for(i=1;i<n;++i) {
    if( i-last_slice == MAX_TASK_SIZE ) {
      // Reached maximum task size
      if( count != 0 ) {
	// Slice is not ordered: create new task
        tasks[*task_n].slice_init = last_slice;
        tasks[*task_n].slice_size = MAX_TASK_SIZE;
	last_slice               = i;
        count                    = 0;
        (*task_n)++;
      } else {
	// Slice was ordered: skip
	last_slice = i;
      }
      continue;
    }

    if( v[i-1] > v[i] ) {
      // Elements out of order
      count++;
      if( count == TASK_TRIGGER ) {
	// Enough out of order elements. Create new task
	tasks[*task_n].slice_init = last_slice;
	tasks[*task_n].slice_size = i-last_slice+1;
	last_slice               = i+1;
	count                    = 0;
	(*task_n)++;
	i++;
      }
      continue;
    }

    if( i == n-1 ) {
      if( count != 0 ) {
        tasks[*task_n].slice_init = last_slice;
        tasks[*task_n].slice_size = n-last_slice;
        last_slice               = i;
        count                    = 0;
        (*task_n)++;
      }
    }
  }

  return tasks;
}
