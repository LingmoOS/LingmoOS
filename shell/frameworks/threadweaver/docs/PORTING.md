# Porting to newer versions

## From KF5 to KF6

To update your application from KF5 to KF6 ThreadWeaver:

* `Collection::stop(JobPointer job)` - argument was removed and now it's just `Collection::stop()`
    1. Replace `collection->stop(job)` with `collection->stop()`
    2. Replace `sequence->stop(job)` with `sequence->stop()`

