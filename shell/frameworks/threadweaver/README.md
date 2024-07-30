# ThreadWeaver

Helper for multithreaded programming

## Introduction

ThreadWeaver is a helper for multithreaded programming.  It uses a job-based
interface to queue tasks and execute them in an efficient way.

You simply divide the workload into jobs, state the dependencies between the jobs
and ThreadWeaver will work out the most efficient way of dividing the work between
threads within a set of resource limits.

See the information on [use cases](@ref usecases) and
[why multithreading can help](@ref multithreading), as well as the usage
section below, for more detailed information.



## Usage

If you are using CMake, you need to have

    find_package(KF6ThreadWeaver NO_MODULE)

(or similar) in your CMakeLists.txt file, and you need to link to
KF6::ThreadWeaver.

ThreadWeaver is a Job queue. It executes jobs in threads it internally manages.
The minimum and maximum number of threads provided by a Weaver is set by the
user. Jobs are regular QObjects, which allows users to connect to the `done()`
signal to be notified when the Job has been executed. The Weaver class provides
objects that handle a number of threads called the inventory. Users usually
acquire a reference to a WeaverInterface object.

Jobs may depend on other jobs. A job will only execute if all jobs it depends
on are already finished. In this, dependencies reorder job execution.  If no
dependencies are declared, jobs are executed in queueing order. Multiple
dependencies are possible, which allows the creation of complex flow graphs
that are automatically executed by the Weaver. It is important, though, to
avoid circular dependencies. Two jobs that depend on each other in both
directions will simply never be executed, since the dependencies will never
resolve.

Threads are created on demand and do not exit until the containing weaver is
deleted. Threads have an eager policy in trying to execute jobs out of the
queue. The managing Weaver blocks them if no jobs are available.

WeaverObservers are used to receive more informative events about the thread
states and job execution. They can be used to provide progress or debugging
information or to implement GUIs to show the thread activity. Observers can be
attached to Weavers and will disconnect automatically when they are deleted.

### Job Execution

In general, jobs are executed in the order they are queued, if they have no
unresolved dependencies. This behaviour can be used to balance I/O, network and
CPU load. The SMIV example shows how this can be done.

### Emitting Signals from Jobs

To notify the application's GUI of progress or other events, it may be
desirable to emit signals from the Job objects that can be connected to the
main thread. Since the job will be executed in another thread, such signals are
delivered asynchronously.

The Job class in the ThreadWeaver library itself contains such a helper class
that can be used as a reference for this approach.

## Porting

Documentation about porting your code to newer versions is described in `docs/PORTING.md`

