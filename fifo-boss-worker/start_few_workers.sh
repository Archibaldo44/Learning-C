#!/bin/bash
for i in {1..5};
do
    ./worker &
done
wait
echo "Done."
