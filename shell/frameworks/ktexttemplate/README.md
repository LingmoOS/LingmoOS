
The KTextTemplate Library
=========================

* About KTextTemplate
* Installation
* Licensing
* Contributing

About KTextTemplate
-------------------

KTextTemplate is a Free Software library written using the [Qt framework](http://code.qt.io).

The goal of KTextTemplate is to make it easier for application developers to
separate the structure of documents from the data they contain, opening the door
for theming and advanced generation of other text such as code.

The syntax uses the syntax of the [Django template system](https://docs.djangoproject.com/en/1.9/ref/templates/language/), and
the core design of Django is reused in KTextTemplate.

    <ul>
    {% for athlete in athlete_list %}
    <li>{{ athlete.name }}{% if athlete.isCaptain %} (C){% endif %}</li>
    {% endfor %}
    </ul>

Part of the design of both template systems is that application developers can
extend the syntax by implementing their own tags and filters. For details of how
to do that, see the [API documentation](http://www.kde.org/apidox/extension.html).

Installation
------------

To build KTextTemplate, you need at least Qt6.0 (with development packages) and CMake 3.16.
Out-of-source builds are recommended:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$prefix ..
    cmake --build .
    cmake --build . --target install

Licensing
---------

KTextTemplate is covered by the GNU Lesser General Public License Version 2.1, or
at your option, any later version. All contributions to KTextTemplate must be
covered by the same license.

The details of the license are in the COPYING file in the source distribution.
