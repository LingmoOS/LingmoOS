## Doing things in a Sequence

The time when an application starts, especially one that needs
to load quite some data, is usually one of contention. Translations
need to be loaded and resources like icons and images initialized. As
the application matures, more and more of such tasks are piled on to
it. It will have to check for updates from a server, and load a
greeting of the day to the user. Eventually, the application will
take ages to load, users will tweet about how they are making coffee
while it comes up, and the programmers will start to find a solution. 

The application will come up a lot faster if it defers as many
tasks as possible while it creates and shows the user interface, and
also takes as many as possible of the startup tasks of an application
off the main thread. The main thread is the one that runs when
`main()` is entered, and in which the user interface lives. Everything
that slows down or intermittendly blocks the main thread may be
experienced by the user as the user interface being sluggish or
hung. This is a common use case where concurrent programming can
help. 

But ... this is also one of the examples where standard thread pools
fail. The startup tasks commonly need to be done in a certain order
and are of different priority, and also should not be all tackled by
the application process at the same time. 
For example, the applications icons and translations may be needed
first and urgently, where the information on available updates can
still be processed a couple of seconds later. There are ways around
this that are rather cumbersome, like using timers to queue up some
tasks later or using chains of functions that queue up new tasks when
one group is done. The following example will illustrate some aspects
of how ThreadWeaver comes with the necessary tools to specify the
order of tasks, on application startup and otherwise. The following
`main()`[^4] function allocates a main widget and an object of type
`ViewController` that takes care of the startup tasks.

@@snippet(HelloInternet/main.cpp,hellointernet-main,cpp)

The example application shows an image that it eventually loads from
the network, and a caption for it. In the constructor of
`ViewController`, the startup operations need to be kicked off. The
operations in this example are 

* to load a placeholder image that is shown while the application
  loads the image and caption from the network,
* to load the post that contains the caption, but only the URL of the
  image to show,
* and then, once the image URL is known, to finally load the image
  from the network and display it.

The application's user interface will be shown right away, even before
step 1 has been completed. Let's assume that the three steps need to
be done in order, not in parallel.

The important aspect is to do as little as possible in the
constructor, considering that it is called from the main
thread. Creating jobs and queueing them is not expensive however, so
the constructor focuses on that and then returns.

@@snippet(HelloInternet/ViewController.cpp,hellointernet-sequence,cpp)

Remember the assumption that the three startup steps have to be
performed in order. The new thing here is that instead of queueing
individual jobs, the constructor creates a `Sequence`, and then adds
jobs to that. A sequence has the jobs performed by the thread pool in
the order they have been added to it. The jobs each simply call a
member function of `ViewController` when being
executed. ThreadWeaver's execution logic guarantees that the next job
is only executed after the previous one has been finished. Because of
that, only one of the member functions will be called at a time, and
they will be called in the order the jobs have been added to the
sequence. 

Since only one of the member functions will be called at a time, there
is no need for further synchronization of access to the member
variables of `ViewController`. This raises the question of how the
controller submits new captions, statuses and images to the main
widget. It would be a mistake to simply call member functions of the
main widget from the methods of `ViewController`, since these are
executed from a worker thread. The controller submits update by using
Qt signals that are connected to corresponding slots in the main
widget. The parameters of the signals are passed by value, not by
reference or pointers, making use of the implicit sharing built into
Qt to avoid copying. This approach relies on the fact that the
reference counting of Qt's implicit-sharing mechanism is thread safe. 

@@snippet(HelloInternet/ViewController.cpp,hellointernet-loadresource,cpp)

The method `loadPlaceholderFromResource()` implements the first step,
to load an image from a resource that acts as a place holder until the
real images has been downloaded. It cheats to appear busy by first
sleeping for a short while. While it does so, the user interface will
already appear to the user, with a blank background. It then emits a
signal to make the main widget show a status message that indicates
the program is downloading the post. 

The method is called from the worker thread that executes the job, not
the main thread. When the signal is emitted, Qt notices that sender and
receiver are not in the same thread at the time, and sends the signal
asynchronously. The receiver will not be called from the thread
executing `loadPlaceholderFromResource()`, instead it will be invoked
from the event loop of the main thread. That means there is no shared
data between the controller and the main widget for processing the
signal, and no further serialization of access to the QString variable
holding the status text is necessary.

Once the method returns and the job executing it completes, the next
job of the sequence will be unlocked. This causes the method
`loadPostFromTumblr()` to be executed by a worker thread. This method
illustrates the convenience built into Qt to process data present in
Open Standard formats (XML, in this case), even though this won't be
discussed here in detail.[^5] If processing the data turns out to be
expensive, the user interface will not be blocked by it, since it
is not performed by the main thread.

@@snippet(HelloInternet/ViewController.cpp,hellointernet-loadpost,cpp)

In case an error occurs, the method invokes another method called
`error()`. `error()` indicates the problem to the user by setting a
status messages in the main widget. But it also apparently aborts the
execution of the sequence, as the code assumes it does not continue
after calling it.

@@snippet(HelloInternet/ViewController.cpp,hellointernet-error,cpp)

`error()` shows a different placeholder image, and emits the status
message to the main widget. It then raises an exception of type
`ThreadWeaver::JobFailed`, which will be caught by the worker thread
executing the current job. The worker thread sets the status of
the job to a failed state. Specific to sequences (because only
sequences know the order of the execution of their elements), this
will cause the sequence to abort the execution of it's remaining
elements. Raising the exception will abort the processing of the job,
but not terminate the worker thread. Leaking any other type of
exception than `ThreadWeaver::Exception` from the `run()` method of a
job is considered a runtime error. The exception will not be caught by
the worker thread, and the application will terminate. 

The example illustrates the steps necessary to perform concurrent
operations in a certain order. It also shows how a specialized
object (`ViewController`, in this case) can handle the data shared
between the sequential operations, how to submit data and status
information back to the user interface, and how to signal error
conditions from job execution.

![Hello Internet](screenshots/HelloInternet.png "The HelloInternet
 example, after downloading the post") 

[^4]: See `examples/HelloInternet` in the ThreadWeaver repository.
[^5]: The example uses the
[Tumblr API version 1](https://www.tumblr.com/docs/en/api/v1). 
