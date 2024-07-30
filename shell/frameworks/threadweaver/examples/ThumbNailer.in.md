## Working title: Everything in moderation (and decorated)

Let's put the features that have been described so far and a few more
that as of yet haven't been mentioned, and create a comprehensive
example program. The example program calculates thumbnails for
images. It will take a number of image files and, in separate steps
implemented as individual jobs, load them from disk, convert them from
raw data to QImages, scale the images to thumbnails, and finally save
them to disk. This problem may not be most imaginative use of concurrent
programming techniques, but it does demonstrate a number of practical
problems. For example, the operations for each single image contain
elements that are file system I/O bound and elements that are CPU
bound. For large numbers of images, it has to deal with a trade-off of
memory usage and CPU utlilization. Less obvious, there are also
expectations on the order of execution of the jobs, so that the
interface provides the user with visible feedback of the progress of
the operations. An application of this kind also should provide
features of load management and reduce it's own generated system load
if the system is "stressed" by other processes. A web server
implementation or a video coding program will have to solve similar
issues to provide optimal throughput without overloading the system. 


