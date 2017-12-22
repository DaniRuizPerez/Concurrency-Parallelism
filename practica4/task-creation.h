#define MAX_TASK_SIZE 100
#define TASK_TRIGGER  10

typedef struct {
  int slice_init;
  int slice_size;
} slice_t;

slice_t * task_creation( double * v, int n, int * task_n );
