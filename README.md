README for `andrelucas/mkhot`
=============================

This is a very simple CPU load generator, suitable for use when stress testing.

_WARNING!_ This will, by design, cause very heavy CPU utilisation. That may cost
you money, shorten the life of your hardware (or someone else's hardware), and
otherwise affect running services.

# Running

```sh
$ docker run andrelucas/mkhot
Starting 4 load-generator thread(s), mode 'wait'
```

With no options, `mkhot` will start one system thread per hardware thread (as
reported via C++ `std::thread::hardware_concurrency()`) which will run a
busy-wait loop. No special instructions, no contention, no system calls - just a spinning loop.

## Options

There are a few options:

```sh
$ docker run -ti andrelucas/mkhot --help
Usage: mkhot [OPTIONS]
Where OPTIONS can be:
  --help                                Display this message
  -i [ --io-load ]                      Perform I/O in the load threads
  -n [ --no-fork ]                      Do not fork() on startup
  -h [ --override-hw-threads ] arg (=4) Override the number of hardware threads
  -t [ --threads-per-core ] arg (=1)    Threads per hardware core/thread

```

The `-i` option specifies an I/O operation should be run in the busy threads
either. This is simply a read of `/dev/zero`; we don't care about _actual_ I/O,
we just want to generate system call.s

The `-h` and `-t` options act as multipliers for the number of threads started.
`-h` is defaulted to the number of hardware threads detected, and `-t`
multiplies the number in `-h` in order to generate more load and contention.
`-t` is especially useful for the I/O load option.

### nofork mode

When running under Docker we end up as PID 1 inside the container, and we don't
get terminal signals. Normally, to avoid this, we'll `fork()` just before running the load threads in order to not be PID 1 any more, and so get CTRL-C etc. send to the process. This means `mkhot` can be stopped from the terminal instead of requiring action via e.g. `docker stop`.

This mode can be turned off with the `-n` option.
