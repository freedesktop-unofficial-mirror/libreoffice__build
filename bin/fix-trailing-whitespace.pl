#!/usr/bin/perl -w

# Reads a patch, and according to what it sees there, it fixes trailing
# whitespace in the files it touches.  To be used in conjuction with
# git diff-index, so it assumes that the patch is 'correct' (but has some
# basic checks to avoid shooting into the leg)
#
# The patch has to be ready for -p0 application
#
# Usage: git diff-index -p --no-prefix --cached HEAD | fix-trailing-whitespace.pl

use strict;
use File::Temp qw/ :mktemp  /;
use File::Copy;

my $patch = $1;
my $file = "";
my %lines = ();
my $line_no = 0;
my $line_max = -1;

# $1 - file to fix
# $2 - list of lines containing whitespace errors
sub fix_whitespace($%) {
    my ( $file, $lines ) = @_;

    # usually we have nothing to do ;-)
    return if ( keys( %lines ) == 0 ||
                $file eq "" ||
                !( $file =~ /\.(c|cpp|cxx|h|hrc|hxx|idl|inl|java|map|mk|MK|pl|pm|pmk|py|sdi|sh|src|tab)/ ) );

    open( IN, "$file" ) || die "Cannot open $file for reading";
    my ( $out, $tmpfile ) = mkstemp( "/tmp/whitespace-fixing-XXXXXX" );

    my $line_no = 1;
    while ( my $line = <IN> ) {
        if ( $lines{$line_no} && $line =~ /^(.*[^ \t])[ \t]+$/ ) {
            print $out "$1\n";
        }
        elsif ( $lines{$line_no} && $line =~ /^[ \t]+$/ ) {
            print $out "\n";
        }
        else {
            print $out $line;
        }
        ++$line_no;
    }
    close( $out );
    close( IN );

    move( $tmpfile, $file ) or die "Cannot move '$tmpfile' to '$file'";
    print "$file\n"
}

# go through the patch and collect lines to fix
while ( my $line = <STDIN> ) {
    if ( $line =~ /^\+\+\+ (.*)/ ) {
        fix_whitespace( $file, \%lines );
        $file = $1;
        %lines = ();
        $line_no = 0;
        $line_max = -1;
    }
    elsif ( $line =~ /^@@ -[0-9]+,[0-9]+ \+([0-9]+),([0-9]+) @@/ ) {
        $line_no = $1;
        $line_max = $line_no + $2;
    }
    elsif ( ( $line_no < $line_max ) && ( $line =~ /^[ +]/ ) ) {
        if ( $line =~ /^\+.*[ \t]$/ ) {
            $lines{$line_no} = 1;
        }
        ++$line_no;
    }
}
fix_whitespace( $file, \%lines );
