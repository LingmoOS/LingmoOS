# Searching For Files {#searching}

Baloo provides a convenient tool called `baloosearch` which can be used to
perform searches. Additionally Baloo is also integrated in popular
applications such as KRunner and Dolphin.

Example:

    $ baloosearch coldplay
    /home/user/Music/Coldplay - Ghost Stories
    /home/user/Music/Coldplay - Ghost Stories/01. Always In My Head.mp3
    /home/user/Music/Coldplay - Ghost Stories/06. Another's Arms.mp3
    /home/user/Music/Coldplay - Ghost Stories/03. Ink.mp3


## Advanced Searches

Baloo offers a rich syntax for searching through your files. Certain
attributes of a file can be searched through.

For example 'type' can be used to filter for files based on their general type

`type:Audio` or `type:Document`

Currently the following types are supported:

* Archive
* Folder
* Audio
* Video
* Image
* Document
  * Spreadsheet
  * Presentation
* Text

These expressions can be combined using `AND` or `OR` and additional parenthesis.

The full list of properties which can be searched is listed below. They are
grouped by file types.

### All Files
* filename
* modified
* mimetype
* tags
* rating
* userComment

### Audio
* BitRate
* Channels
* Duration
* Genre
* SampleRate
* TrackNumber
* ReleaseYear
* Comment
* Artist
* Album
* AlbumArtist
* Composer
* Lyricist


### Documents
* Author
* Title
* Subject
* Generator
* PageCount
* WordCount
* LineCount
* Language
* Copyright
* Publisher
* CreationDate
* Keywords

### Media
* Width
* Height
* AspectRatio
* FrameRate

### Images
* ImageMake
* ImageModel
* ImageDateTime
* ImageOrientation
* PhotoFlash
* PhotoPixelXDimension
* PhotoPixelYDimension
* PhotoDateTimeOriginal
* PhotoFocalLength
* PhotoFocalLengthIn35mmFilm
* PhotoExposureTime
* PhotoFNumber
* PhotoApertureValue
* PhotoExposureBiasValue
* PhotoWhiteBalance
* PhotoMeteringMode
* PhotoISOSpeedRatings
* PhotoSaturation
* PhotoSharpness
* PhotoGpsLatitude
* PhotoGpsLongitude
* PhotoGpsAltitude

