# KCompletion

Powerful completion framework, including completion-enabled lineedit and combobox.

## Introduction

When typing filenames, email addresses and other text where the user often wants
to select from existing data (including what they previously typed) rather than
enter anything wholly original, users often find it helpful if they only need to
type the first few characters, and then have the application offer them a set of
choices or attempt to finish off what they were typing. Email clients, shells
and "open file" dialogs often provide this functionality.

This framework helps implement this in Qt-based applications. You can use one of
the completion-ready widgets provided by this framework, or integrate it into
your application's other widgets directly.

## Usage

The easiest way to get started is to use a KComboBox, KHistoryComboBox or
KLineEdit. If you want to integrate completion into other parts of the user
interface, you can use KCompletion to manage and select the possible
completions.

