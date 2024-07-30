#!/bin/sh

for i in *.dia; do echo $i; dia --export=$i.png --filter=png --nosplash $i; done

#for i in *.dia; do echo $i; dia --export=$i.png --export-to-format=png --nosplash $i; done

