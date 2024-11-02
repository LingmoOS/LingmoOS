# Peony Qt VFS' implemention

## Framework
Peony Qt's framework is based on gvfs, which has been integrated in glib2.0.

In gvfs design, user can register it own virtual filesystem in application with unique schema. The theory of custom vfs is based on interface framework of gvfs.

If you have understand how Peony::FileEnumerator works, you will know that explor don't care if the file is virtrual, because it uses same method which provided by GFileIface.

## Make a custom vfs
Just like the default search vfs which in this directory does. To make a custom vfs in your application, you should:

- use g_vfs_register_uri_scheme() register a unique schema
- implement your virtual file class with GFileIface as interface

g_vfs_register_uri_scheme() is the entry of a custom vfs. It will help us find or create the correct handler when getting a GFile instance by uri.

The implement is very flexiable, you might not have to implement all interface of GFileIface unless you have to use one or more method ineeded.

Default search vfs shows us how to make a custom search vfs and combine the vfs with Peony::FileEnumerator. The important point is make sure the vfs file handler use its custom enumerator which can parse the search uri and handle it correctly. There are 3 method must be overrided in vfs enumerator which derived from GFileEnumerator:
- next_file
- next_files_async
- next_files_finish

## Improve the search vfs in explor's rules

The defaul search vfs provided by explor is low efficiency and waste resources. I hope somebody could provide a vfs that obay the uri parsing rules to replace the original one.

The rule of parsing searching uri is:
- use "&" as seperator
- use "keyword"="varient" to descrip the preference.
- use "," to split varient if varient is splitable for the keyword.
- aviliable keywords:
   - name_regexp -- varient type string
   - content_regexp -- type string
   - use_regexp -- 0 or 1
   - recursive -- 0 or 1
   - search_uris -- not null, splitable, type string