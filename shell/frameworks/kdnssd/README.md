# KDE DNS-SD

Network service discovery using Zeroconf

## Introduction

KDNSSD is a library for handling the DNS-based Service Discovery Protocol
(DNS-SD), the layer of [Zeroconf](http://www.zeroconf.org) that allows network
services, such as printers, to be discovered without any user intervention or
centralized infrastructure.


## Usage

If you are using CMake, you need to have

```cmake
find_package(KF6DNSSD NO_MODULE)
```

(or similar) in your CMakeLists.txt file, and you need to link to KF6::DNSSD.

If you are writing an application that wants to discover services on the
network, use KDNSSD::ServiceBrowser.  You can also find available service types
using ServiceTypeBrowser.

If you want to announce the availability of a service provided by your
application, use KDNSSD::PublicService.

KDNSSD::DomainBrowser allows you to find domains (other than the local one)
recommended for browsing or publishing to.

Note that KDNSSD::ServiceBrowser::isAvailable() provides information about the
availability of the services provided by this library generally, not just for
browsing services.


## More information

More information about DNS-SD can be found in the [online
documentation][appledocs] for Apple's implementation of Zeroconf,
[Bonjour](https://developer.apple.com/bonjour/).

[appledocs]: https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/NetServices/Articles/about.html


