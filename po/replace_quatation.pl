#!/usr/bin/perl

use strict;
use warnings;
use Tie::File;

# global
my @po_list = `ls *.po`;

# for replacing "
my $dest_char1 = "\\\\\"";
my $dest_char2 = "\\\"";
my $quatation_char = "\"";

# function
sub replace_quatation
{
    my ($po_file) = @_;
    my @po_file_line = $po_file;

    foreach (@po_file_line) {
        tie my @filearray, 'Tie::File', $_ or die "Couldn't open file $_, $!";       
        foreach my $line(@filearray) {
            if (($line =~ /msgstr\ \"(.*)\"/)) {
                my $new_line = $1;

                # quatation check
                # there is no qutation, continue
                if (index($new_line, "\"") != -1) {
                    printf "replace: $new_line\n";
                    # equalizing to "
                    $new_line =~ s/$dest_char1/$quatation_char/g;

                    # replace " with \"
                    $new_line =~ s/$quatation_char/$dest_char2/g;

                    # write new line
                    $line = "msgstr\ \"$new_line\"";
                }
            }
        }
        untie @filearray;
    }
}

sub replace_another
{
    my ($po_file) = @_;
    my @po_file_line = $po_file;

    foreach (@po_file_line) {
        tie my @filearray, 'Tie::File', $_ or die "Couldn't open file $_, $!";       
        foreach my $line(@filearray) {
            if (($line =~ /msgstr\ \"(.*)\"/)) {
                my $new_line = $1;

		if (index($new_line, "\\") != -1) {
                    printf "replace: $new_line\n";

                    # replace \n with <br> 
                    $new_line =~ s/\\/ /g;

                    # write new line
                    $line = "msgstr\ \"$new_line\"";
                }
            }
        }
        untie @filearray;
    }
}

sub replace_br
{
    my ($po_file) = @_;
    my @po_file_line = $po_file;

    foreach (@po_file_line) {
        tie my @filearray, 'Tie::File', $_ or die "Couldn't open file $_, $!";       
        foreach my $line(@filearray) {
            if (($line =~ /msgstr\ \"(.*)\"/)) {
                my $new_line = $1;

		if (index($new_line, "<br>") != -1) {
                    printf "replace: $new_line\n";

                    # replace <br> with \n 
                    $new_line =~ s/<br>/\\n/g;

                    # write new line
                    $line = "msgstr\ \"$new_line\"";
                }
            }
        }
        untie @filearray;
    }
}

sub replace_driver_str
{
    my ($po_file) = @_;
    my @po_file_line = $po_file;

    foreach (@po_file_line) {
        tie my @filearray, 'Tie::File', $_ or die "Couldn't open file $_, $!";       
        foreach my $line(@filearray) {
            if (($line =~ /msgstr\ \"(.*)\"/)) {
                my $new_line = $1;

		if (index($new_line, ":\\") != -1) {
                    printf "replace: $new_line\n";

		    if( index($new_line, ":\\\\") == -1 ) {
			# replace <br> with \n 
	                $new_line =~ s/:\\/:\\\\/g;

	 	        # write new line
	                $line = "msgstr\ \"$new_line\"";
		    }
                }
            }
        }
        untie @filearray;
    }
}

# main
foreach (@po_list) {
    my $file = $_;
    chomp($file);
    replace_quatation($file);
    replace_driver_str($file);
    #replace_another($file);
}
