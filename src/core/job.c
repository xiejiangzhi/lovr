#include "job.h"
#include <stdatomic.h>
#include <threads.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKERS 64
#define MAX_JOBS 1024

struct job {
  job* next;
  atomic_uint done;
  union { fn_job* fn; char* error; };
  void* arg;
};

static struct {
  job jobs[MAX_JOBS];
  thrd_t workers[MAX_WORKERS];
  uint32_t workerCount;
  job* head;
  job* tail;
  job* pool;
  cnd_t hasJob;
  mtx_t lock;
  bool quit;
} state;

static thread_local jmp_buf catch;

// Must hold lock, this will unlock it, state.head must exist
static void runJob(void) {
  job* job = state.head;
  state.head = job->next;
  if (!job->next) state.tail = NULL;
  mtx_unlock(&state.lock);

  if (setjmp(catch) == 0) {
    fn_job* fn = job->fn;
    job->error = NULL;
    fn(job, job->arg);
  }

  job->done = true;
}

static int workerLoop(void* arg) {
  for (;;) {
    mtx_lock(&state.lock);

    while (!state.head && !state.quit) {
      cnd_wait(&state.hasJob, &state.lock);
    }

    if (state.quit) {
      break;
    }

    runJob();
  }

  mtx_unlock(&state.lock);
  return 0;
}

bool job_init(uint32_t count) {
  mtx_init(&state.lock, mtx_plain);
  cnd_init(&state.hasJob);

  state.pool = state.jobs;
  for (uint32_t i = 0; i < MAX_JOBS - 1; i++) {
    state.jobs[i].next = &state.jobs[i + 1];
  }

  if (count > MAX_WORKERS) count = MAX_WORKERS;
  for (uint32_t i = 0; i < count; i++, state.workerCount++) {
    if (thrd_create(&state.workers[i], workerLoop, (void*) (uintptr_t) i) != thrd_success) {
      return false;
    }
  }

  return true;
}

void job_destroy(void) {
  state.quit = true;
  cnd_broadcast(&state.hasJob);
  for (uint32_t i = 0; i < state.workerCount; i++) {
    thrd_join(state.workers[i], NULL);
  }
  cnd_destroy(&state.hasJob);
  mtx_destroy(&state.lock);
  memset(&state, 0, sizeof(state));
}

job* job_start(fn_job* fn, void* arg) {
  for (;;) {
    mtx_lock(&state.lock);

    if (state.pool) {
      break;
    } else if (state.head) {
      runJob();
    } else { // Might not be a job to do if worker count is bigger than pool size
      mtx_unlock(&state.lock);
      thrd_yield();
    }
  }

  job* job = state.pool;
  state.pool = job->next;

  if (state.tail) {
    state.tail->next = job;
    state.tail = job;
  } else {
    state.head = job;
    state.tail = job;
    cnd_signal(&state.hasJob);
  }

  job->next = NULL;
  job->done = false;
  job->fn = fn;
  job->arg = arg;

  mtx_unlock(&state.lock);
  return job;
}

void job_abort(job* job, const char* error) {
  size_t length = strlen(error);
  job->error = malloc(length + 1);
  if (job->error) memcpy(job->error, error, length + 1);
  else job->error = strdup("Out of memory");
  longjmp(catch, 22);
}

void job_vabort(job* job, const char* format, va_list args) {
  int length = vsnprintf(NULL, 0, format, args);
  job->error = malloc(length + 1);
  if (job->error) vsnprintf(job->error, length + 1, format, args);
  else job->error = strdup("Out of memory");
  longjmp(catch, 22);
}

void job_wait(job* job) {
  while (!job->done) {
    mtx_lock(&state.lock);

    if (state.head) {
      runJob();
    } else {
      mtx_unlock(&state.lock);
      thrd_yield();
    }
  }
}

const char* job_get_error(job* job) {
  return (char*) job->error;
}

void job_free(job* job) {
  mtx_lock(&state.lock);
  if (job->error) free(job->error);
  job->next = state.pool;
  state.pool = job;
  mtx_unlock(&state.lock);
}
