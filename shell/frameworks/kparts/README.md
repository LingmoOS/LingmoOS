# KParts

Plugin framework for user interface components

## Introduction

This library implements the framework for KDE parts, which are
elaborate widgets with a user-interface defined in terms of actions
(menu items, toolbar icons).


## Usage

If you are using CMake, you need to have

    find_package(KF6Parts NO_MODULE)

(or similar) in your CMakeLists.txt file, and you need to link to KF6::Parts.

The main class is KParts::Part.  This, or one of its subclasses, is what authors
of parts implement, and how users of parts interact with them.

Applications wishing to embed a part need to inherit their main window from
KParts::MainWindow and provide a so-called shell GUI, which provides a basic
skeleton GUI with part-independent functionality/actions.

Some KParts applications won't be specific to a given part, but expect
to be able to embed, for instance, all types of viewers out there. For this
the basic functionality of any viewer has been implemented in
KParts::ReadOnlyPart, which viewer-like parts should inherit from.
The same applies to KParts::ReadWritePart, which is for editor-like parts.

You can add actions to an existing KParts app from "outside", defining
the code for those actions in a shared library. This mechanism is
obviously called plugins, and implemented by KParts::Plugin.

For a complete, and very simple, example of how to use KParts to display
any kind of file (i.e. making a generic viewer), see the documentation for
KParts::PartLoader::instantiatePartForMimeType().


