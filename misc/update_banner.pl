#!/usr/bin/perl -w
use File::Basename;
use Cwd 'abs_path';
use File::Find;

chdir(dirname(abs_path(__FILE__))) or die "Cannot chdir.\n";

my $banner = read_file('banner.txt');
$banner =~ s/^\s+|\s+$//sg;
$banner = "\n" . $banner . "\n\n";

my $banner_cpp = read_file('banner_cpp.txt');
$banner_cpp =~ s/^\s+|\s+$//sg;
$banner_cpp = $banner . $banner_cpp . "\n\n";

chdir("..");
find(sub {
    my $f = $_;
    my ($comment, $c);
    if ($File::Find::name =~ m{/(t|misc)/}) {
        return;
    } elsif ($f =~ /\.(pl|pm)$|README.txt/) {
        $comment = $banner;
        $c = "#@";
    } elsif ($f =~ /\.(cpp|h)$/) {
        $comment = $banner_cpp;
        $c = "//@";
    } else {
        return;
    }
    $comment =~ s/^/$c /mg;
    $comment =~ s/[ \t]+$//mg;
    my $code = read_file($f);
    $code =~ s{^ (?:(\#!.*?\n))? (?: \Q$c\E \n ( \Q$c\E [^\n]*\n )+ \Q$c\E \n )? \n* }{ ($1||"") . $comment . "\n" }sgex;
    write_file($f, $code);
}, ".");

sub read_file {
    my ($f) = @_;
    open(local *F, $f) or die "Cannot open $f: $!\n";
    local $/;
    my $txt = <F>;
    $txt =~ s/\r//sg;
    return $txt;
}

sub write_file {
    my ($f, $txt) = @_;
    open(local *F, ">", $f) or die "Cannot write to $f: $!\n";
    print F $txt;
}
