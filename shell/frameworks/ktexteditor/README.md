# KTextEditor

Full text editor component

## Introduction

KTextEditor provides a powerful text editor component that you can embed in your
application, either as a KPart or using the KF6::TextEditor library (if you need
more control).

The text editor component contains many useful features, from syntax
highlighting and automatic indentation to advanced scripting support, making it
suitable for everything from a simple embedded text-file editor to an advanced
IDE.

## Usage

If you are using CMake, you need to have

```cmake
find_package(KF6TextEditor)
```

(or similar) in your CMakeLists.txt file, and you need to link to KF6::TextEditor.

After that, you can use KTextEditor::Editor to create an editor instance, and
use that to manage KTextEditor::Document instances.

```cpp
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

// get access to the global editor singleton
auto editor = KTextEditor::Editor::instance();

// create a new document
auto doc = editor->createDocument(this);

// create a widget to display the document
auto view = doc->createView(yourWidgetParent);
```

See the documentation for these classes for more information.

## Licensing

Contributions to KTextEditor shall be licensed under [LGPLv2+](LICENSES/LGPL-2.0-or-later.txt).

All files shall contain a proper "SPDX-License-Identifier: LGPL-2.0-or-later" identifier inside a header like:

```cpp
/*
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
```

## Further Documentation

- @ref kte_design
- @ref kte_port_to_5
- @ref kte_guidelines

