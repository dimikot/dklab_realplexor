#!/bin/bash
cd ../..
perl dklab_realplexor.pl | cat &
trap "kill `top -d1 -b -c -n1 | egrep 'realplexor[.]pl' | perl -ne '/^\s*(\d+)/ and print qq{$1 }'`; exit" INT 
while :; do top -b -c -n1 | egrep 'PI[D]|realplexor[.]pl' | egrep -v '[/]sh'; sleep 2; done
