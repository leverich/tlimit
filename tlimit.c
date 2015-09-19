#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>

int    time_limit      = 30; // default 30 seconds
int    verbose         = 0;

pid_t target;
pid_t pid;

double tv_to_double(struct timeval *tv) {
  return tv->tv_sec + (double) tv->tv_usec / 1000000;
}

void double_to_tv(double val, struct timeval *tv) {
  long long secs = (long long) val;
  long long usecs = (long long) ((val - secs) * 1000000);

  tv->tv_sec = secs;
  tv->tv_usec = usecs;
}

double get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv_to_double(&tv);
}

// arguments
// -v
// -t <time limit>
// app [<args> ..]

void usage() {
  printf(
         "Usage: tlimit [-v] [-t <time limit>\n"
         "              app [<args> ..] | PID\n"
         );
  exit(0);
}

int main(int argc, char **argv, char **envv) {
  int c;

  double now_time, stop_time;

  int status, ret;
  struct itimerval itv;

  while (1) {
    c = getopt(argc, argv, "t:hv");
    if (c == -1) break;

    switch (c) {
    case 'v': verbose = 1; break;
    case 't':
      time_limit = atoi(optarg);
      
      if (time_limit <= 0 || time_limit > 360000) {
        printf("Throttle amount must be between (0,360000].\n");
        exit(-1);
      }

      break;

    default:
      usage();
      break;
    }
  }

  if (argc == optind) usage();

  // See if the supplied app is actually a pid
  pid = atoi(argv[optind]);
  if (!(pid > 0 && pid < 65536)) pid = -1;

  if (pid != -1 && kill(pid, 0)) {
    if (errno == ESRCH) {
      printf("PID %d not found.\n", pid);
      exit(-1);
    }
  }

  if (pid != -1) { // user supplied pid
    target = pid;
  } else { // spawn target
    target = getpid();

    pid_t child = fork();

    if (!child) { // child
      if (fork()) exit(0); // the child needs to fork again so it can detach entirely from the parent
      setsid(); // start a new session, yee haw

    } else { // parent
      waitpid(child, NULL, 0);
      execvp(argv[optind], argv+optind);
    }
  }

  stop_time = get_time() + time_limit;

  while(1) {
    now_time = get_time();

    if (now_time > stop_time) {
      //      kill(target, SIGTERM);
      kill(target, SIGINT);
    }

    if (kill(target, 0))
      if (errno == ESRCH) break;

    sleep(1);
  }

  return 0;
}
