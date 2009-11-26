#!/bin/sh

cd ../..
export dir
for dir in t/servertest/ api/*/t/; do 
	(cd $dir && echo "Executing tests in $dir..." && pear run-tests)
done

# Why PHPT test framework? Because it is extremely simple. 
# I just love it.
