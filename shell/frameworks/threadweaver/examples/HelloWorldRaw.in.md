## Hello World! with queueing multiple jobs

The first example showed nothing that would have required multiple
threads to print _Hello World!_, and also did not mention anything
about at what time jobs get deleted. Object life span is of course a
crucial questions when programming in C++. So of what type is the 
value that is returned by `make_job` in the first example? 

The returned object is of type `JobPointer`, which is a
`QSharedPointer` to `Job`. When `make_job` is executed, it allocates a
`Job` that will later execute the C++ lambda function, and then embeds
it into a shared pointer. Shared pointers count references to the
object pointer they represent, and delete the object when the
last reference to it is destroyed. In a way, they are single-object
garbage collectors. In the example, the new job is immediately handed
over to the queue stream, and no reference to it is kept by
`main()`. This approach is often called "fire-and-forget jobs". 
The queue will process the job and forget about it when it
has been completed. It will then definitely get deleted automatically,
even though the programmer does not necessarily know exactly when. It
could happen (and in the case of ThreadWeaver jobs commonly does)
deeply in the bowels of Qt event handling when the last event holding
a reference to the job gets destroyed. The gist of it is that from the
programmers point of view, it is not necessary to keep a reference to
a job and delete it later. With that in mind, no further memory
management is required in the HelloWorld example, and the program is
complete.

Fire-and-forget jobs are not always the right tool. For example, if
a job is retrieving and parsing some data, the application needs to
access the data once the job is complete. For that, the programmer
could implement a custom job class.

@@snippet(HelloWorldRaw/HelloWorldRaw.cpp,sample-helloworldraw-class,cpp)

The `QDebugJob` class simply prints a message to `qDebug()` when it is
executed. To implement such a custom job class, it is inherited from
`ThreadWeaver::Job`. By overloading the `run()` method, the "payload",
the operation performed by the job, is being defined. The parameters
to the run method are the job as the queue sees it, and the thread
that is executing the job. The first parameter may be surprising. The
reason that there may be a difference between the job that the queue
sees and `this` is that jobs may be decorated, that means wrapped in
something else that waddles and quacks like a job, before being
queued. How this works will be explained later, what is important to
keep in mind for now is not to assume to always find `this` in the
queue.

@@snippet(HelloWorldRaw/HelloWorldRaw.cpp,sample-helloworldraw-main,cpp)

This time, in the `main()` function, four jobs in total will be
allocated. Two of them as local variables (j1 and j2), one (j3)
dynamically and saved in a `JobPointer`, and finally j4 is allocated
on the heap with `new`. 
All of them are then queued up for execution in one single
command. Wait, what? Right. Local variables, job pointers and raw
pointers are queued the same way and may be mixed and matched using
the stream operators. When a local variable is queued, a special
shared pointer will be used to hold it which does not delete the
object when the reference count reaches zero. A `JobPointer` is simply
a shared pointer. A raw pointer will be considered a new object and
automatically wrapped in a shared pointer and deleted when it goes out
of scope. Even though three different kinds of objects are handed over
to the stream, in all three cases the programmer does not need to put
special consideration into memory management and the object life
cycles. 

Now before executing the program, pause for a minute and think about
what you expect it to print. 

~~~~
World!
This is...
Hello
ThreadWeaver!
~~~~

Four jobs are being queued all at the same time, that is when the
`stream()` statement closes. Assuming there is more than one worker
thread, the order of execution of the jobs is undefined. The strings
will be printed in arbitrary order. In case this comes as a surprise,
it is important to keep in mind that by default, there is no relation
between jobs that defines their execution order. This behaviour is in
line with how thread pools normally work. In ThreadWeaver, there are
ways to influence the order of execution by declaring dependencies
between them or aggregating multiple jobs into collections or
sequences. More on that later. 

Before the end of `main()`, the application will block and wait for
the queue to finish all jobs. This was not needed in the first
HelloWorld example, so why is it necessary here? As explained there,
the global queue will be destroyed when the `QCoreApplication` object
is destroyed. If `main()` would exit before j1 and j2 have been
executed, it's local variables including j1 and j2 would be
destroyed. In the destructor of `QCoreApplication` the queue would
wait to finish all jobs, and try to execute j1 and j2, which have
already been destructed. Mayhem would ensue. When using local
variables as jobs, make sure that they have been completed before
destroying them. The `finish()` method of the queue guarantees that it
no more holds references to any jobs that have been executed.

