#!/usr/bin/perl -w

my $file = $ARGV[0];
die "Usage:\n  $0 file-name.out\n" if !$file || $file !~ /^(.*)\.out$/s;

my $phpt = $1 . ".phpt";
my $out = `cat $file` or die "Cannot read $file\n";
my $c = `cat $phpt | egrep -B 100000 "^[-][-]EXPECT"`;
$c =~ /^--EXPECT--\s*\Z/ms or die "Cannot grep EXPECT from $phpt\n";

open(F, ">$phpt");
print F $c;
print F $out;
close(F);

system("pear run-tests $phpt");