Use Cases         {#usecases}
=========

ThreadWeaver provides a solution to a number (but not all)
multithreading problems. Let's have a look:

- How do you implement a single operation that takes a lot of CPU power, but is
  hard to handle in chunks (Example: scale an image) when you want your GUI to
  remain responsive? Encapsulate the operation in a class object derived from
  Job instead of a method and put it in the static instance (Weaver::instance).
  Connect to the Job's done() signal to receive a notification when it is
  completed.
- How do you implement a CPU intensive operation while time-critical operations
  are executed (load and decode an MP3 while another one is fed to the audio
  device): Implement both the file loading operation and the play operation in
  a job and queue both at the same time. There is also a synchronous sleep
  method in the Job class if a job needs to be delayed for a number of
  milliseconds after it is taken over by a thread.
- How do you implement many small operations where the cost of each individual
  one is hard to estimate (loading two hundred icons from an NFS drive): Create
  a common or a number of specialized job classes. Queue all of them. Either
  connect to the individual done signal to process every item when it has been
  finished or connect to Weaver::jobsDone() to receive a notification when all
  of the jobs are done.
- How do you implement an operation with complex control flow and dependencies
  in the execution order (load, parse and display an HTML document with
  embedded media): Create jobs for the individual steps you need to perform.
  Try to split the whole operation in as many independent, parallelizable parts
  as possible. Now declare the execution dependencies. A job will only be
  executed when all jobs it depends on are finished. This way every individual
  operation will be executed as soon as it becomes possible. Connect to the
  final jobs done() signal to be notified when all parts of the whole
  operations have been executed. If necessary (if there is more than one final
  object), create a dummy job object that depends on all of them to have one
  central end point of execution.

As you can see, ThreadWeaver can provide solutions for simple, but
also for complex cases. For an example on how job dependencies can be
modeled and used to create elegant, streamlined solutions for control
flow modeling, see the Simple Multithreaded Image Viewer (SMIV) example
in the Tests directory.

ThreadWeaver focuses on operations that can be implemented in
Jobs. To create solutions where a thread is supposed to constantly run
to perform an ongoing operation that, for example, spans the whole
application live, it has to be verified that it is the right
approach. It is possible to add very long or neverending operations to
the queue, but it will constantly bind a thread to that operation. It
might still make sense to use ThreadWeaver, as the creation, handling
and destruction of the threads is already taken care of, but the minimum
inventory size (thread count) should be increased accordingly to provide
for enough threads to process the other jobs.

