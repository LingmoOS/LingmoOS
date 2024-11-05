# Model and ModelItem

As we should display a large number element in the file manager's view, we should not handle them directly. Model is desgin to help us doing that.

There are 2 main types model item in explorer. Item in directory view and item in side bar.

## FileItem
FileItem is the absctract item class contract with FileItemModel.

The different from FileInfo to FileItem is that FileItem has concept of children and parent. This makes FileItem instance has a tree struction and can represent a tree item in a view(non-tree as well). Other different is that FileItem instance is not shared. You can hold many FileItem instances crosponding to the same FileInfo, but they are allocated in their own memory space. Every FileItem instance which has children will aslo support monitoring. When find the children of the item, it will start a monitor for this directory. 

## FileItemModel
FileItemModel is the model of FileItem.

This class supplies the common interface to QAbstractItemView based view classes. A FileItem instance must bind to a model instance, so that it could tell the view how to show its or its children's data through the model. 

You can learn more model/view programming at Qt's official document and example.

## FileItemProxyFilterSortModel
This class is use for sorting and filtering the FileItemModel instance. Every proxy model have a mapping to a FileItemModel.
FileItemModel is disordered and non-filterable, that means we can't sort the files by name or other order and filter the hidden files.

With this interface, it's much easier to implement the sorting and filtering.

## item in side bar
Item in side bar is similar to item in directory view. The main differences between 2 items are:
- side bar item is extensionable by impling SideBarAbstractItem interface.
- side bar item is more lightweight, it doesn't hold a FileInfo instance.