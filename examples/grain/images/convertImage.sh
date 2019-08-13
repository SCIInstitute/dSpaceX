#!/bin/bash
for i in {1..400}
do
   convert $i.png -define png:color-type=6 $i.png
done