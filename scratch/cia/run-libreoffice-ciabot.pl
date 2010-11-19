#!/usr/bin/perl -w

if ( ! -d 'bootstrap' ) {
    print STDERR "Not a directory with libreoffice repos!\n";
    exit 1;
}

sub error($) {
    my ( $message ) = @_;
    print STDERR "$message\n";
}

sub get_branches() {
    my %branches;
    if ( open REFS, "git show-ref |" ) {
        while ( <REFS> ) {
            chomp;
            if ( /^([^ ]*) refs\/remotes\/origin\/(.*)/ ) {
                if ( $2 ne 'HEAD' ) {
                    $branches{$2} = $1;
                }
            }
        }
        close REFS;
    }
    else {
        error( "Cannot call git show-ref." );
    }

    return \%branches;
}

sub report($$$) {
    my ( $repo, $old_ref, $new_ref ) = @_;
    my %old = %{$old_ref};
    my %new = %{$new_ref};

    foreach my $key ( keys %new ) {
        my $branch_name = $key;
        $branch_name = '' if ( $branch_name eq 'master' );

        my $old_head = $old{$key};
        my $new_head = $new{$key};

        if ( defined( $old_head ) ) {
            if ( $old_head ne $new_head ) {
                if ( open COMMITS, "git rev-list $new_head ^$old_head | tac |" ) {
                    while ( <COMMITS> ) {
                        chomp;
                        print "Sending report about $_ in $key\n";
                        qx(libreoffice-ciabot.pl $repo $_ $branch_name)
                    }
                    close COMMITS;
                }
                else {
                    error( "Cannot call git rev-list." );
                }
            }
        }
        else {
            # TODO at some stage, we could play with git merge-base, and
            # report everything etc. but... ;-)
            print "Sending report about $_ in a newly created branch $key\n";
            qx(libreoffice-ciabot.pl $repo $new_head $branch_name)
        }
    }
}

print "Checking for changes in the libreoffice repo & sending reports to CIA.vc.\n";

@all_repos = (
    "artwork",
    "base",
    "bootstrap",
    "build",
    "calc",
    "components",
    "extensions",
    "extras",
    "filters",
    "help",
    "impress",
    "l10n",
    "libs-core",
    "libs-extern",
    "libs-extern-sys",
    "libs-gui",
    "postprocess",
    "sdk",
    "testing",
    "ure",
    "writer"
);

chomp( my $cwd = `pwd` );

my %old_ref;
foreach $repo (@all_repos) {
    chdir "$cwd/$repo";
    $old_ref{$repo} = get_branches();
}

while ( 1 ) {
    foreach $repo (@all_repos) {
	chdir "$cwd/$repo";

	# update
	qx(git fetch origin);
	my $new_ref = get_branches();

	# report
	report( $repo, $old_ref{$repo}, $new_ref );
	$old_ref{$repo} = $new_ref;
    }

    # check every 5 minutes
    sleep 5*60;
}
