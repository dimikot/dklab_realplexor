#!/usr/bin/perl -w

my $file = $ARGV[0];
die "Usage:\n  $0 file-name.exp\n" if !$file || $file !~ /^(.*)\.exp$/s;

my $out = $file; $out =~ s/\.exp$/.out/s;
system("clear; diff $file $out");

scalar <STDIN>;