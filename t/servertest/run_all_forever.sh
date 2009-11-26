#!/bin/sh

cd ../..
export dir
export arg=$1

while :; do
	for dir in t/servertest/ api/*/t/; do 
		name=`echo "$dir" | perl -pe "s{/$}{}sg; s{/}{_}sg"`
		out=/tmp/dklab_realplexor.test/`date +"%Y-%m-%d_%H:%M:%S"`/$name
		(cd $dir && echo "Executing tests in $dir..." && pear run-tests $arg)
		if [ -f $dir/run-tests.log ]; then
			mkdir -p $out
			(cd $dir; cp *.log *.out *.exp [0-9]*.php *.diff $out)
		fi
	done
done
