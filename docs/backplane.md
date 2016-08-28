Bolo Backplane
==============

This document discusses the mid- and low-level architecture of the
core server backplane, specifically with respect to the movement
of data through memory.

I am writing this document primarily to convince myself that an
architecture exists, that it is sane, and that it lends itself to
performance optimization well.  The last thing we need is for the
core of the monitoring system to be a bottleneck on processing
metric data.

Block Diagram
-------------

At its most reduced, bolo is a system of transforming inputs into
outputs:

```

   +-------+     +--------+     +--------+
   | input | --> |  bolo  | --> | output |
   +-------+     +--------+     +--------+

```

Of course, it's a bit more complicated than this.  For starters,
we have multiple inputs, in different formats, listening on
different ports.  We have a native listener, an OpenTSDB listener,
a Graphite listener, etc.

We also have multiple outputs; bolo subscribers over TSDP,
OpenTSDB for Grafana visualization, etc.  Some of our outputs may
even loop back over to the inside of the diagram, as is the case
in most native subscribers, which report metadata about their
internal workins back to the core.

At the middle of the diagram, which I have pithily labeled 'bolo',
a lot of processing and internal data movement, manipulation and
munging goes on, some of which delays the flow of data from
_input_ to _output_.


Orchestration
-------------

If all of this diagram is contained in the same address space, we
definitely want to multi-thread.  A thread of workers can handle
incoming and outgoing requests, freeing up CPU to be used where
CPU can best be used, without swapping out the whole process.

That means we need a system of orchestration.

POSIX Threads (pthreads) locking should be sufficient for our
purposes.  We'll start by implementing a rough cut of the locking
strategy, and then run a load across the system and look for race
detection and idle states.

Race conditions are bad because they often don't manifest as
erratic behavior, but can.  When they do, reproduction of the bug
is difficult if not outright impossible.  Valgrind has a
[DRD][drd] utility for race detection.

Lock idling is where parts of the system (i.e. _certain threads_)
are idle, waiting on a lock.  That's not intrinsically bad, but it
does cut into performance via degraded throughput.  If we can
analyze these idle states, we might be able to redesign the data
pipeline to eliminate or minimize them.  I have not yet found a
tool for this.





Pluggable Modules for Input / Output vs. Standalone Exec w/IPC?
Static Modules for processing to organize code?
