This document will evolve over time to indicate what goals and use cases
lingmo-appstore targets at the moment.

Primary goals
=============

 * Allow people to find apps by browsing or search:
    - a specific app that they're looking for, or
    - apps in a particular category, or with particular functionality that they require
 * Allow people to effectively inspect and appraise apps before they install them (screenshots, descriptions, ratings, comments, metadata)
 * Allow people to view which apps are installed and remove them
 * Present a positive view of the app ecosystem
    - Reinforce the sense that there are lots of high quality apps
    - Encourage people to engage with that ecosystem, both as users and as contributors
    - When browsing, present and promote the best apps that are available
    - Facilitate accidental discovery of great apps
 * Handle software updates. Make software updates as little work for users as possible. To include: apps,  OS updates (PackageKit, eos, rpm-ostree), firmware
 * Support multiple software repositories, defined by both the distributor and users.
    - Show which repos are configured. Allow them to be added/removed.
    - Handle cases where the same app can be installed from multiple sources.

Secondary goals
===============

 * OS upgrades
 * Hardware driver installation
 * Input method installation
 * Respond to application queries for software (apps, codecs, languages)
 * Offline and metered connections
 * OS updates end of life
 * App end of life

Non-goals
=========

 * Not a package manager front-end
 * Not all repos are equal
 * Not all apps are equal
