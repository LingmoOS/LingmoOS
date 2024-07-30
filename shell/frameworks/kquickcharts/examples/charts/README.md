Charts Gallery Example
======================

This example both demonstrates the different charts and their usage and can be
used as a development tool to quick check if certain things still work.

Command Line Arguments
----------------------

These are mainly intended as development helpers.

* --page <page>
  Open the application at the specified page. <page> can be either a page title
  or a short identifier. Currently these are `pie`, `line` and `continuous`.
* --api <api>
  Run with the specified "API". This will make sure we use the specified
  version/profile/api combination and is intended to test how things work when
  running on a device that would use that combination by default. The possible
  options for <api> are:
  * core45
    Run with OpenGL 4.5, using the Core profile. This will use different shaders
    from the default, suffixed with _core.
  * compat45
    Run with OpenGL 4.5, using the Compatibility profile. Apparently Qt still
    wants to use the _core suffixed shaders for this though.
  * compat21
    Run with OpenGL 2.1, no profile. This is also the default version used,
    since we require GLSL 1.20 at minimum.
  * es
    Run with OpenGL ES 2.0. This does not use different shaders, but does
    require all stored variables to have a precision qualifier specified, among
    other differences.
