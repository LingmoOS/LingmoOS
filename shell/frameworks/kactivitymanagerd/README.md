# KActivities

Core components for the KDE Activity concept

## Introduction

When a user is interacting with a computer, there are three main areas of
contextual information that may affect the behaviour of the system: who the user
is, where they are, and what they are doing.

*Activities* deal with the last one. An activity might be "developing a KDE
application", "studying 19th century art", "composing music" or "watching funny
videos". Each of these activites may involve multiple applications, and a single
application may be used in multiple activities (for example, most activities are
likely to involve using a web browser, but different activities will probably
involve different websites).

KActivities provides the infrastructure needed to manage a user's activites,
allowing them to switch between tasks, and for applications to update their
state to match the user's current activity. This includes a daemon, a library
for interacting with that daemon, and plugins for integration with other
frameworks.

## Usage

Most applications that wish to be activity-aware will want to use
KActivities::Consumer to keep track of the user's current activity, and
KActivities::ResourceInstance to notify the activity manager of resources the
user has accessed (this is not necessary for resources accessed via KIO, as a
plugin is provided to do that automatically).

The other classes available in the API are primarily intended for use by the
workspace to allow the user to view and manage available activities.
