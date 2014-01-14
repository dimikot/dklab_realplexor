#!/usr/bin/perl -w
use File::Basename;
use Cwd 'abs_path';
use File::Find;

chdir(dirname(dirname(abs_path(__FILE__)))) or die "Cannot chdir.\n";
find(sub {
    /\.git|jquery/ and return;
    /\.(pl|pm|cpp|h|phpt|txt|py|conf|js|jst|sh|html|htm|init|htpasswd|php)$/ or return;
    print $File::Find::name . "\n";
    my $perms = (stat $_)[2] & 0777;
    system "cat $_ | expand -t4 | perl -pe 's{ +\$}{}sg; s{\\r}{}sg;' > /tmp/expanded; cmp $_ /tmp/expanded >/dev/null || mv -f /tmp/expanded $_";
    chmod $perms, $_;
}, ".");
