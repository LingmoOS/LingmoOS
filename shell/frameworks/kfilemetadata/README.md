# KFileMetaData

## Introduction

KFileMetaData provides a simple library for extracting the text and metadata
from a number of different files. This library is typically used by file
indexers to retrieve the metadata. This library can also be used by applications to write metadata.

## Using the library

In order to use the library you must implement your own ExtractionResult
class. Instances of this class will be passed to every applicable plugin and
they will populate with the information.

For convenience a SimpleExtractionResult class has been provided which stores all the
data in memory and allows it to be introspected later. Most clients *should*
implement their own ExtractionResult as the data can get quite large when
extracting the text content from very large files.

The library also supports plugins that write back data.

## Extracting Metadata from a file

This requires us to create a ExtractionPluginManager class, fetch the extractor
plugins which are applicable for that file, and then pass the instance of
ExtractionResult to each Extractor.

A simple test example called `dump.cpp` has been written.

## Writing Metadata to a file

This will require calling the WriterCollection class's fetchWriters() method with the mimetype of the file that needs to be written to. This method will return a list of writers, and to actually write metadata, a call to the write() method is required. The write() method accepts an instance of the WriteData class, which stores a mapping of the properties to be written and their values.

## Writing a custom file extractor

The Metadata is extracted with the help of Extraction Plugins. Each plugin
provides a list of mimetypes that it supports, and implements the extraction
function which extracts the data and fills it in an ExtractionResult.

Most of the common file types are already provided by the library.

Extractors should typically avoid implementing any logic themselves and should
just be wrappers on top of existing libraries.

## Writing a custom metadata writer

The writeback framework uses an approach similar to the extraction framework. Each writer plugin supports a list of mimetypes and implements the write function that takes in a WriteData object as input.

### Adding data into an `ExtractionResult`

The ExtractionResult can be filled with (key, value) pairs and plain text. The
keys in these pairs typically correspond to a predefinied property. The list
of properties is defined in the `properties.h` header. Every plugin should
use the properties defined in this header. If a required property is missing
then it should be added to this framework.

The ExtractionResult should also be given a list of types. These types are
defined in the `types.h` header. The correspond to a higher level overview
of the files which the user typically expects.

## Writing an external plugin

Extractors and Writers can also be written in other languages and installed into the system,
and KFileMetaData will be able to find them and use them.

An external plugin must be an independently executable file (a binary,
script with a hashbang line with the executable permission set, a batch file or
cmd script, etc). They must be located within libexec directory.

KFileMetaData will wrap each external extractor with an instance of the ExternalExtractor class, and every writer with `ExternalWriter`. The application will be free to choose any of the plugins returned by WriterCollection or ExtractorCollection.

Every external plugin will be placed within a directory in libexec/kf5/kfilemetadata/externalextractors. Every plugin shall have a manifest.json file that specifies the mimetypes that the plugin supports and the main executable file. A sample manifest file is located at src/writers/externalwriters/example/manifest.json.

Both kinds of plugins accept the target file as an argument.

### Writing an external extractor

Extractors take JSON formatted input specifying the input mimetype, and return JSON output with the extracted properties. The JSON output also indicates any errors that might have occurred. Calls to the extractor are blocking, hence there is a time limit for how long they can run.

### Writing an external writer

Writers take in the properties to be changed via stdin and return JSON output with the success value of the write operation. Calls to the writerare blocking, hence there is a time limit for how long they can run.

## Links
- Mailing list: <https://mail.kde.org/mailman/listinfo/kde-devel>
- IRC channel: #kde-devel on Libera Chat
