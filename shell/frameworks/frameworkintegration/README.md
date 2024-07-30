# Framework Integration

Integration of Qt application with KDE workspaces

## Introduction

Framework Integration is a set of plugins responsible for better integration of
Qt applications when running on a KDE Lingmo workspace.

Applications do not need to link to this directly.

## Components

### KF6Style

The library KF6Style provides integration with KDE Lingmo Workspace
settings for Qt styles.

Derive your Qt style from KStyle to automatically inherit various
settings from the KDE Lingmo Workspace, providing a consistent user
experience. For example, this will ensure a consistent single-click
or double-click activation setting, and the use of standard themed
icons.

### FrameworkIntegrationPlugin

FrameworkIntegrationPlugin provides extra features to other KDE
Frameworks to integrate with KDE Lingmo.

It currently provides an addon to KMessageBox which stores settings
for asking users the same question again.

