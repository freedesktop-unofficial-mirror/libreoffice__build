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
                my $ret = system("git rev-parse -q --verify $new_head^2 >/dev/null");
                if ($ret != 0) {
                    # not a merge commit, announce every commit
                    if ( open COMMITS, "git rev-list $new_head ^$old_head | tac |" ) {
                        while ( <COMMITS> ) {
                            chomp;
                            print "Sending report about $_ in $key\n";
                            if (!$test) {
                                qx(libreoffice-ciabot.pl $repo $_ $branch_name)
                            } else {
                                print "libreoffice-ciabot.pl '$repo' '$_' '$branch_name'\n";
                            }
                        }
                        close COMMITS;
                    }
                    else {
                        error( "Cannot call git rev-list." );
                    }
                } else {
                    # just process the merge commit itself
                    print "Sending report about $new_head in $key\n";
                    if (!$test) {
                        qx(libreoffice-ciabot.pl $repo $new_head $branch_name)
                    } else {
                        print "libreoffice-ciabot.pl '$repo' '$new_head' '$branch_name'\n";
                    }
                }
            }
        }
        else {
            # Report the newest commit which is not in master
            if ( open COMMITS, "git rev-list -n 1 $new_head ^refs/remotes/origin/master |" ) {
                while ( <COMMITS> ) {
                    chomp;
                    print "Sending report about $_ in $key (newly created branch)\n";
                    if (!$test) {
                        qx(libreoffice-ciabot.pl $repo $_ $branch_name)
                    } else {
                        print "libreoffice-ciabot.pl '$repo' '$_' '$branch_name'\n";
                    }
                }
                close COMMITS;
            }
            else {
                error( "Cannot call git rev-list." );
            }
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

$test = 0;

if ($test) {
    @all_repos = ("test");
}

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

    if (!$test) {
        # check every 5 minutes
        sleep 5*60;
    } else {
        print "Hit enter to report...\n";
        <STDIN>;
    }
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
