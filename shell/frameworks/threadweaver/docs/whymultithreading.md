Why Multithreading?     {#multithreading}
===================

In the past, multithreading has been considered a powerful tool that
is hard to handle (some call it the work of the devil). While there may
be some truth to this, newer tools have made the job of the software
developer much easier when creating parallel implementations of
algorithms. At the same time, the necessity to use multiple threads to
create performant applications has become more and more
clear. Technologies like Hyperthreading and multiple core processors can
only be used if processors have to schedule processing power to
multiple, concurrently running processes or threads.

Event driven programs especially bear the issue of latency, which is
more important for the user's impression of application performance than
other factors. But the responsiveness of the user interface relies
mainly on the ability of the application to process events, an ability
that is much limited if the application is executing processing
power expensive, lengthy operations. This leads, for example, to delayed
or sluggish processing of necessary paint events. Even if this does not influence the total time necessary to perform the operation the
user requested, it is annoying and not state of the art.

There are different approaches to solve this issue. The crudest one
may be to process single or multiple events while performing a lengthy
operation, which may or may not work sufficiently, but is at least sure
to ruin all efforts of Separation of Concerns. Concerns can simply not
be separated if the developer has to intermingle instructions with event
handling where the kind of events that are
processed are not known in advance.

Another approach is to use event-controlled asynchronous
operations. This is sufficient in most cases, but still causes a number
of issues. Any operation that carries the possibility of taking a long
time or blocking may still stop event processing for a while. Such risks
are hard to assess, and especially hard to test in laboratory
environments where networks are fast and reliable and the system I/O
load is generally low. Network operations may fail. Hard disks may be
suspended. The I/O subsystem may be so busy that transferring 2 kByte may
take a couple of seconds.

Processing events in objects that are executed in other threads is
another approach. There are other issues that come with parallel
programming, but it does ensure the main event loop returns as soon as
possible. Usually this approach is combined with a state pattern to
synchronize the GUI with the threaded event processing.

Which one of these approaches is suitable for a specific case has to
be assessed by the application developers. There is no silver
bullet. All have specific strengths, weaknesses and issues. The
ThreadWeaver library provides the means to implement multithreaded job
oriented solutions.

To create performant applications, the application designers have to
leverage the functionality provided by the hardware platform as much as
possible. While code optimizations only lead to slight improvement,
application performance is usually determined by network and I/O
throughput. The CPU time needed is usually negligible. At the same time,
the different hardware subsystems usually are independent in modern
architectures. Network, I/O and memory interfaces can transfer data all
at the same time, and the CPU is able to process instructions while all
these subsystems are busy. The modern computer is not a traditional
uniprocessor (think of GPUs, too). This makes it necessary to use all
these parallel subsystems at the same time as much as possible to
actually use the possibilities modern hardware provides, which is very
hard to achieve in a single thread.

Another very important issue is application processing
flow. Especially GUI applications do not follow the traditional
imperative programming pattern. Execution flow is more network-like,
with chunks of code that depend on others to finish processing before
they can touch their data. Tools to represent those
networks to set up your applications order of execution are rare, and
usually leave it to the developers to code the execution order of the
instructions. This solutions are usually not flexible and do not adapt
to the actual usage of the CPU nodes and computer
subsystems. ThreadWeaver provides means to represent code execution
dependencies and relies on the operating systems scheduler to actually
distribute the work load. The result is an implementation that is very
close to the original application semantics, and usually improved
performance and scalability in different real-life scenarios.

The more tasks are handled in parallel, the more memory is
necessary. There is a permanent CPU - memory tradeoff which limits the
number of parallel operations to the extent where memory that needs to
be swapped in and out slows down the operations. Therefore memory usage
needs to be equalized to allow the processors to operate without being
slowed down. This means parallel operations need to be scheduled to a
limit to balance CPU and memory usage. ThreadWeaver provides the means
to do that.

In general, ThreadWeaver tries to make the task of creating
multithreaded, performant applications as simple as
possible. Programmers should be relieved of synchronization, execution
dependency and load balancing issues as much as possible. The API
tends to be clean, extensible and easy to understand.
