Dependency diagrams {#depdiagrams}
===================

## Introduction

In most case, you just want to call `kapidox` with the
"--dependency-diagrams" option. However, if you want finer-grained control over
diagram generation or want to work on its code, read on.

## depdiagram tools

Dependency diagram generation is done using two tools: `src/depdiagram-prepare`
and `src/depdiagram-generate`. The way you use it is as follows.

### 1. Prepare dot files

You need to prepare Graphviz dot files for all frameworks with
`src/depdiagram-prepare`. You can prepare dot files for all frameworks at once
using:

    depdiagram-prepare --all ~/src/frameworks ~/dots

This will generate many .dot files in ~/dots.

Or you can prepare dot files for a single framework with:

    depdiagram-prepare --single ~/src/frameworks/myframework ~/dots

### 2. Generate the diagrams

You can now generate the dependency diagrams with `src/depdiagram-generate`.
This tool accepts a list of dot files and outputs a combined dot file to stdout.

Here is how to generate a dependency diagram for all the frameworks:

    depdiagram-generate ~/dots/*.dot | dot -Tpng > kf5.png

The diagram might be very hard to read though. For complex diagrams, you may
want to pipe the output through the `tred` tool:

    depdiagram-generate ~/dots/*.dot | tred | dot -Tpng > kf5.png

You can also generate the diagram for one particular framework using the
"--framework" option:

    depdiagram-generate ~/dots/*.dot --framework kcrash | tred | dot -Tpng > kcrash.png

To include Qt libs, use the "--qt" option:

    depdiagram-generate ~/dots/tier*/*/*.dot --framework kcrash --qt | tred | dot -Tpng > kcrash.png

And to include targets within the framework, use the "--detailed" option:

    depdiagram-generate ~/dots/*.dot --framework kcrash --detailed | tred | dot -Tpng > kcrash.png


## Useful 3rd-party tools

`tred`, mentioned in the previous section, reduces clutter in dot files.

`xdot` can be used instead of `dot` to display the graph:

    depdiagram-generate ~/dots/*.dot --framework kcrash --qt | tred | xdot


## Generating all diagrams at once

You can use the `depdiagram-generate-all` tool to generate diagrams for all
frameworks at once:

    depdiagram-generate-all ~/dots ~/pngs

This command creates two pngs for each framework: "$framework.png" and
"$framework-simplified.png" (same diagram, ran through `tred`). It also creates a
diagram for all the frameworks, named "kf5.png".
