#!/usr/bin/perl -w

use File::Copy;
use File::Basename;

sub get_search_paths()
{
    my @paths = ();
    my @search = split /:/, $options{'PATCHPATH'};

    for my $stem (@search) {
	push @paths, "$patch_dir/$stem";
    }
#    print "Search paths are '@paths'\n";
    return @paths;
}

sub find_patch_file($)
{
    my $file = shift;
    my $file_path = '';

    $file =~ m/\.diff$/ || die "Patch file '$file' is not a .diff";

    for my $path (get_search_paths()) {
	if (-f "$path/$file") {
	    $file_path = "$path/$file";
	    last;
	}
    }

    -f $file_path || die "\n\n** Error ** - Can't find file $file " .
	"in patch path '" . $options{'PATCHPATH'} . "'\n\n\n";

    return $file_path;
}

sub do_patch {
    my $patch = shift;
    my $base_cmd = shift;
    my $patch_file = basename($patch);
    my $cmd_output = "";
    my $cmd_suffix = "";

    print "$patch_file: testing..."; 

    if ($quiet) 
        { $cmd_output = " > /dev/null " }

    $cmd_pipe = " < $patch";
    my $cmd = "( $base_cmd --dry-run ".$cmd_output.$cmd_suffix.")".$cmd_pipe;

    print "$cmd\n" unless $quiet;
    system ($cmd) && die "Testing patch $patch_file failed.";

    $cmd = "( $base_cmd > /dev/null".$cmd_suffix.")".$cmd_pipe;
    if($quiet)
        { print "applying..." }
    else
        { print "$cmd\n" }

    system ($cmd) && die "Failed to patch $patch_file\n" .
			 "- this is often caused by duplicate sections in a patch.\n".
			 "you will need to manually reverse this patch\n";
    print "done.\n";
}

sub slurp {
    my $from = shift;
    my $save = $/;
    my $FROM;

    undef $/;
    open ($FROM, "<$from");
    # slurp whole file in one big string
    my $content = <$FROM>;
    close ($FROM);
    $/ = $save;

    return $content;
}

sub select_subset ($$);

sub select_subset ($$)
{
    my $subsets = shift;
    my $tag = shift;
    my @nodes = ();

    if (defined $subsets->{$tag}) {
	@nodes = @{$subsets->{$tag}};
    }
	
    if (!@nodes) {
	return @nodes;
    }

    my $subtag;
    my @result = @nodes;

    my $count = @nodes;
    for $subtag (@nodes) {
	my @subset = select_subset($subsets, $subtag);
	push @result, @subset;
    }

    return @result;
}

sub canonicalize_milestone($)
{
    my $milestone = shift;
    $milestone =~ m/([0-9]+)$/ || return $milestone;
    my $stem = $milestone;
    my $num = $1;
    $num = sprintf("%04d", $num);
    $stem =~ s/([0-9]+)$//;
    $milestone = $stem . $num;
#    print "Canonicalize milestone to '$milestone'\n";
    return $milestone;
}

sub milestone_cmp($$)
{
    my $a = shift;
    my $b = shift;
    $a = canonicalize_milestone($a);
    $b = canonicalize_milestone($b);
    return $a cmp $b;
}

sub same_mws($$)
{
    my $a = shift;
    my $b = shift;
    $a =~ s/-.*//;
    $b =~ s/-.*//;
#    print "same mws? '$a' vs '$b'\n";
    return $a eq $b;
}

sub rules_pass($)
{
    my $original_rule = shift;
    my $rule = $original_rule;

    while ($rule ne '') {
	my $lastrule = $rule;
#	print "verify rule '$rule' vs '$tag'\n";

	if ($rule =~ /[\!\<\=\>]+\s*(\S+)/ && !same_mws($tag, $1) ) {
	    # ignore milestones from other MWSes
#	    print "ignoring rule '$rule' for '$tag'\n";
	    $rule =~ s/[\!\<\=\>]+\s*(\S+)//;
	} else {
	    # equal to (==)
	    if ($rule =~ s/\=\=\s*(\S+)// ) {
		   if ( milestone_cmp ($tag, $1) != 0 ) { return 0; } 
	    }
	    # non-equal to (!=)
	    elsif ($rule =~ s/\!\=\s*(\S+)// ) {
	           if ( milestone_cmp ($tag, $1) == 0 ) { return 0; }
	    }
	    # less than or equal (<=)
	    elsif ($rule =~ s/\<=\s*(\S+)// ) {
	           if ( milestone_cmp ($tag, $1) > 0 ) { return 0; }
	    }
	    # less than (<)
	    elsif ($rule =~ s/\<\s*(\S+)// ) {
	    	   if ( milestone_cmp ($tag, $1) >= 0 ) { return 0; }
	    }
	    # greater than or equal (>=)
	    elsif ($rule =~ s/\>=\s*(\S+)// ) {
	           if ( milestone_cmp ($tag, $1) < 0 ) { return 0; }
	    }
	    # greater than (>)
	    elsif ($rule =~ s/\>\s*(\S+)// ) {
	    	   if ( milestone_cmp ($tag, $1) <= 0 ) { return 0; }
	    }
	}

	$rule =~ s/^\s*//;

	$lastrule eq $rule && die "Invalid syntax in rule: $original_rule\n";
    }
    return 1;
}

sub filter_targets($)
{
    my $tlist = shift;
    my @targets;
    my $rules_passed;

    $tlist =~ m/([^!<>=]*)(.*)/;
    my $rules = $2;
    my $targets = $1;
    $targets =~ s/\s*$//;

    @targets = split /\s*,\s*/, $targets;
    $rules_passed = rules_pass ($rules);

#    if ($rules_passed) {
#	printf "Rule '$rules' passed ['@targets']\n";
#    } else {
#	printf "Rule '$rules' failed ['@targets']\n";
#    }

    return $rules_passed, @targets ;
}

sub patched_files($) {
    my ( $file ) = @_;
    
    my @lines = split (/\n/, $file);
    my @dest;

    foreach $line (@lines) {
        if ( $line =~ /\+\+\+ ([^\s]*)/ ) {
            push @dest, $1;
        }
    }

    return @dest;
}

sub is_dependent_patch($$) {
    my ( $patchedref, $patch ) = @_;
    
    my @files = patched_files ($patch);

    foreach $file (@files) {
        if ( exists $patchedref->{$file} ) {
            return 1;
        }
    }

    return 0;
}

sub scan_patch_entry($$$$)
{
    my ($line,$line_num,$section_issue,$section_owner) = @_;
    
    my %patch_entry = ();
    $patch_entry{'issue'} = $section_issue;
    # the developer must not be initialized to $section_owner because it helps us to catch syntax errors
    $patch_entry{'developer'} = 'unknown';

    foreach (split(/,\s*/, $line)) {
	if (/i#([0-9]+)/) {
	    # more (duplicated) issues can be fined, we take the latest one
	    $patch_entry{'issue'} = $1;
	} elsif (/\w+#([0-9]+)/) {
	    # valid syntax but unknown bugzilla => ignore
	} else {
	    ($patch_entry{'developer'} eq 'unknown') || die "Syntax error at $apply_list, line $line_num, element \"$_\"\n" .
				                            "Note that the one developer name is already defined as \"$patch_entry{'developer'}\".\n";
	    $patch_entry{'developer'} = $_;
	}
    }
    
    if ($patch_entry{'developer'} eq 'unknown') {
	$patch_entry{'developer'} = $section_owner;
    }
    
    return \%patch_entry;
}

sub scan_section_issue($$)
{
    my ($line,$line_num) = @_;

    my $issue = '';
    foreach (split(/,\s*/, $line)) {
	if (/^i#([0-9]+)/) {
	    # more (duplicated) issues can be fined, we take the latest one
	    $issue = $1;
	} elsif (/\w+\#([0-9]+)/) {
	    # valid syntax but unknown bugzilla => ignore
	} else {
	    die "Syntax error at $apply_list, line $line_num, element \"$_\".\n";
	}
    }
    return $issue;
}

sub add_developer_info
{
    my $temp_file = `mktemp $apply_list.XXXX`;
    chomp $temp_file;

    open (PatchListIn, "$apply_list") || die "Can't open $apply_list: $!\n";
    open (PatchListOut, ">$temp_file") || die "Can't open $temp_file: $!\n";

    my $section_owner = 'unknown';

    # this is necessary because the later cvs call
    my $pwd_orig = `pwd`;
    chdir $patch_dir;

    print "This command adds missing names of the responsible developers for each patch.\n\n";
    
    print "IMPORTANT: This command works correctly only if you call it on the ooo-build\n" .
          "cvs snapshot (not the released one). To run this command, you need cvs access\n".
	  "to ooo-build.\n\n";
    
    print "HINT: You might want to use the commands ssh-agent and ssh-add before this one.\n" .
	  "Then you need not enter the password for each patch.\n\n\n";


    print "Guessing developer names for patches:\n";

    while (<PatchListIn>) {
	chomp;
	my $add_info='';
	
	
        if (/^\s*#.*/) {
	    # comment
	} elsif (/^\s*SectionOwner\s*\=\>\s*(.*)/) {
	    # SectionOwner
	    $section_owner = $1;
	    $section_owner = 'unknown' unless ($section_owner);
#	    print "New section owner => $section_owner\n";
	} elsif (/^\s*\[\s*(.*)\]/) {
	    # new section
	    $section_owner = 'unknown';
	} elsif (/^\s*([^\#,\s]+.diff)\s*,?\s*(.*)?$/) {
	    # patch entry
	    my $patch_name = $1;
	    my $tmp = $2;
	    $tmp = '' unless ($tmp);

	    # ignore patches extracted from cws; they often have an outside developer
	    # they are ignored in the statistic, so...
	    unless ($patch_name =~ /cws\-/) {

		my $patch_entryp = scan_patch_entry($tmp, $., "0", $section_owner);

		if ($patch_entryp->{'developer'} eq 'unknown' ) {
		    my $patch_file_path = find_patch_file ($patch_name);
		    $patch_file_path =~ s|$patch_dir/||;
		    print "$patch_file_path ... ";
		    my $cvs_developer = `cvs annotate -F $patch_file_path 2>/dev/null | cut -d\\( -f2 | cut -d' '   -f1 | sort | uniq -c | sort --numeric-sort | tail -n 1 | cut -c 9-`;
		    chomp $cvs_developer;
		
		    ($cvs_developer) || die "\n" .
					    "Error: The deloper name cannot be detected from cvs\n\n" .
		                    	    "Hint: Try the following two commands to check the cvs output:\n" .
					    "\tcd $patch_dir\n" .
					    "\tcvs annotate -F $patch_file_path\n";
		
		    $add_info=", $cvs_developer";
		    print "$cvs_developer\n";
		}
	    }
	}	
	print PatchListOut "$_" . "$add_info" . "\n";
    }

    print "done\n";

    close PatchListIn;
    close PatchListOut;

    rename ($temp_file, $apply_list) || die "Can't rename $temp_file to $apply_list: $!\n";
    chmod (0644, $apply_list) || die "Can't chmod of $apply_list: $!\n";

    chdir $pwd_orig;
}


sub list_patches(@) {
    my ( @distros ) = @_;

    my @Patches = ();
    foreach $distro ( @distros ) {
	@Patches = ( @Patches, list_patches_single( $distro ) );
    }

    return @Patches;
}

sub list_patches_single {

    sub checkTerminal {
	my $subsets = shift;
	my $referenced = shift;
	my $distro = shift;

	if (defined $referenced->{$distro} ||
	    !defined $subsets->{$distro}) {
	    print "Error: you must use a terminal node for distro:\n";
	    my $ref;
	    my @terminals;
	    print "    One of: ";
	    for $ref(keys %{$subsets}) {
		if (!defined $referenced->{$ref}) {
		    push @terminals, $ref;
		}
	    }
	    print join ', ', sort (@terminals);
	    print "\n";
	    exit (1);
	}
    }

    my $forDistro = shift;
    my @Patches = ();

    open (PatchList, "$apply_list") || die "Can't find $apply_list";

    my @targets=();
    my %subsets=();
    my @selected_subsets=();
    my %referenced=();
    my $rules_passed = 0;
    my $section_owner = 'unknown';
    my $section_issue = '';
    my $first_section_found = 0;
    while (<PatchList>) {
	    # First, process # as the comment delimier only if it is the first non-blank character
	    # It will be processed one more times after we check for all entries with patch numbers,
	    # where '#' has another meaning
            s/^\s*#.*//;
            chomp;
	    s/\r//; # Win32

	    # SectionIssue
            if (/\s*SectionIssue\s*\=\>\s*(.*)$/) {
		$section_issue = scan_section_issue($1, $.);
#		print "New section issue => $section_issue\n";
		next;
	    }
	    
	    # patch entry
	    if (/^\s*([^,\#\s]+.diff)\s*,?\s*(.*)?$/) {
		my $patch_name = $1;
		my $tmp = $2;
		$tmp = '' unless ($tmp);

#		print "Processing patch entry: $_\n";
	    
		$unfiltered_patch_list{$patch_name} = scan_patch_entry($tmp, $., $section_issue, $section_owner);

		if (defined $unfiltered_patch_list{$patch_name}->{'targets'}) {
		    for $set (@ {$unfiltered_patch_list{$patch_name}->{'targets'}}) {
			unless (grep /^$set$/i, @targets) {
			    push @ {$unfiltered_patch_list{$patch_name}->{'targets'}}, $set;
			}
		    }
		} else {
#		    print "Initial targets for patch: $patch_name: @targets\n";
		    $unfiltered_patch_list{$patch_name}->{'targets'} = [ @targets ];
		}
	    
		my $set;
		my $match = 0;
		if ($rules_passed) {
		    for $set (@targets) {
			if (grep /^$set$/i, @selected_subsets) {
			    $match = 1;
			}
		    }
		}
		if (!$match) {
#		    print "%subsets: @targets: skipping '$_'\n";
		    next;
		}

		push @Patches, find_patch_file ($patch_name);
		next;
	    }

	    # Finally, process '#' as the commnet delimiter everywhere
            s/\s*\#.*//;
	    s/\s*$//;
	    s/^\s*//;
            $_ eq '' && next;

	    # continuation
	    if (defined $lastline) {
		$_ = $lastline . $_;
		undef $lastline;
	    }
	    if (m/\\\S*$/) {
		s/\\//g;
		$lastline = $_;
		next;
	    }

	    # new section
            if (/\[\s*(.*)\]/) {
                my $tmp = $1;
                $tmp =~ s/\s+$//;
#               print "Target: '$tmp'\n"; 
		($rules_passed, @targets) = filter_targets($tmp);
		$section_owner = 'unknown';
		$section_issue = '';
		$first_section_found = 1;
                next;
            }

	    # SectionOwner; it must be recognized before general options, see below
            if (/\s*SectionOwner\s*\=\>\s*(.*)/) {
		$section_owner = $1;
		$section_owner = 'unknown' unless ($section_owner);
		($section_owner =~ /[\s,]+\S+/) && die "Syntax error at $apply_list, line $.\n" .
						       "Note that that only one developer name can be defined by SectionOwner.\n";
#		print "New section owner => $section_owner\n";
		next;
	    }

	    # General options XXX = YYY
            if (/\s*([_\S]+)\s*=\s*(.*)/) {
		$options{$1} = $2;
		print "$1 => $2\n" unless $quiet;
		# this rule helps to find strange syntax errors
		($first_section_found) && die "Syntax error at $apply_list, line $.\n" .
					      "Note that all options must be defined before the first section.\n" .
					      "You might have misspelled the SectionOwner or SectionIssue tag, so it is not \n" .
					      "recognized and is taken for a general option.\n";
		next;
	    }

            if (/\s*([_\S]+)\s*:\s*(.*)/) {
		my $key = $1;
		my @value = (split /\s*,\s*/, $2);
		$subsets{$key} = [@value];
		@selected_subsets = select_subset (\%subsets, $forDistro);
#		print "selected sets: @selected_subsets\n" unless $quiet;
		# update terminality map
		my $ref;
		for $ref(@value) {
		    $referenced{$ref} = 1;
		}
                next;
            }

	    die "Syntax error at $apply_list, line $.\n";
    }
    close (PatchList);

    checkTerminal (\%subsets, \%referenced, $forDistro);

    return @Patches;
}

sub applied_patches_list
{
    my @patches;
    
    foreach (glob($applied_patches."/???-*")) { 
	m/\~$/ && next; # backup file.
	push @patches, $_;
    }

    return @patches;
}

sub check_tag {
    my $mws_found = 0;
    for my $oldest (split /\s/, $options{'OLDEST_SUPPORTED'}) {
	if ( same_mws( $tag, $oldest ) ) {
	    if ( milestone_cmp($tag, $oldest) < 0 ) {
		die "\n\n** Error ** - Tag '$tag' " .
		    "is too old, please use at least '$oldest'\n\n\n";
	    }
	    $mws_found = 1;
	}
    }
    if ( ! $mws_found ) {
	print "Warning: Nothing known about tag '$tag', " .
	    "please update 'OLDEST_SUPPORTED'.\n";
    }
}

sub apply_patches {

    my @Patches = list_patches (@distros);

    print "\n" unless $quiet;

    for $opt (@required_opts) { 
        defined $options{$opt} || die "Required option $opt not defined"; 
    }

    check_tag();

    if( ! -d $applied_patches ) {
        mkdir $applied_patches || die "Can't make directory $patch_dir: $!";
    }

    my %existing_patches;

    foreach (applied_patches_list()) {
        my $file = basename $_;

        $file =~ s/^([0-9]{3})-//;
        $existing_patches{$file} = $_;
    }

    my %obsolete_patches = %existing_patches;
    foreach $patch (@Patches) {
	delete $obsolete_patches{basename($patch)};
    }

    my @to_apply;
    my @to_unapply;
    my %patched;

    foreach $a (keys %obsolete_patches) {
	$patch = $obsolete_patches{$a};

        unshift @to_unapply, $patch;
        foreach $pf ( patched_files (slurp ($patch)) ) {
            $patched{$pf} = 1;
        }

	delete $existing_patches{$a};
    }

    foreach $patch (@Patches) {
        my $patch_file = basename($patch);
        
        my $is_applied = 0;
        
        if( exists $existing_patches{$patch_file} ) {
            my $applied_patch = $existing_patches{$patch_file};
            my $patch_content = slurp($patch);
            my $applied_content = slurp($applied_patch);
            if (length ($patch_content) == length ($applied_content) &&
                $patch_content eq $applied_content &&
                !is_dependent_patch (\%patched, $applied_content))
            {
                print "$patch_file already applied, skipped\n";
                $is_applied = 1;
            }
            else {
                unshift @to_unapply, $applied_patch;
                foreach $pf ( patched_files( $applied_content ) ) {
                    $patched{$pf} = 1;
                }
            }
            delete $existing_patches{$patch_file};
        }

        if (!$is_applied) {
            push @to_apply, $patch;
        }
    }

    foreach $patch ( sort { $b cmp $a } @to_unapply ) {
        print "\n" unless $quiet;
        print "Unapplying patch $patch\n";
        do_patch $patch, $base_cmd." -R";
        unlink $patch || die "Can't remove $patch $!";
    }

    my $patch_num = 0;
    foreach (applied_patches_list()) {
        my $file = basename $_;

        if ( $file =~ /^([0-9]{3})-/ ) {
            my $num = $1;
            $num =~ s/^0*//;
            
            $patch_num = $num if ( $num > $patch_num );
        }
    }
    $patch_num++;

    foreach $patch (@to_apply) {
        my $patch_file = basename($patch);
        print "\n" unless $quiet;
        do_patch $patch, $base_cmd;

        my $patch_copy = sprintf("%s/%03d-%s", $applied_patches, $patch_num++, $patch_file);

        print "copy $patch_file -> $patch_copy\n" unless $quiet;

        copy($patch, $patch_copy) || die "Can't copy $patch to $patch_copy $!";
    }

    if (keys %existing_patches) {
	die "Error - leftover obsolete patches\n";
    }
}

sub remove_patches {
    my @Patches = ();

    -d $applied_patches || return;

    foreach $patch_file (reverse (applied_patches_list())) {
        print "\nRemoving ".basename($patch_file)."...\n" unless $quiet;
        do_patch $patch_file, $base_cmd;
        unlink $patch_file;
    }
    rmdir $applied_patches
}

sub export_series {
    my @Patches = list_patches (@distros);

    for my $patch (@Patches) {
	$patch =~ s/^\Q$patch_dir\E\/..\///;
	print "$patch -p0\n";
    }
}

sub is_old_patch_version()
{
    my $Patch;
    my $ver_line;
    my $is_old = 1;

    open ($Patch, "patch --version|") || die "Can't run patch: $!";
    $ver_line = <$Patch>;
    $ver_line =~ m/\s+(\d+)\.(\d+)\.(\d+)/ || die "Can't get patch version\n";
    if ($1 >= 2 && $2 >= 5 && $3 >= 9) {
	$is_old = 0;
    }
    
    if ($is_old) {
	print "Old patch version - pruning LFs\n";
    }
    return $is_old;
}


sub scan_unused($$)
{
    my $dir = shift;
    my $patches = shift;
    my $dirh;
    my $warned = 0;

    opendir($dirh, $dir) || die "Can't open $dir: $!";
    while (my $file = readdir ($dirh)) {
	$file =~ /^\./ && next;     # hidden
	$file =~ /\.diff$/ || next; # non-patch

	if (!defined $patches->{$file}) {
	    if (!$warned) {
		print "Warning: unused files in $dir\n";
		$warned++;
	    }
	    print "\t$file\n";
	}
    }
    closedir($dirh);
}

sub check_for_unused ($)
{
    my $patches = shift;

    for my $path (get_search_paths()) {
	scan_unused ($path, $patches);
    }
}

sub print_statistic_no_issue ($)
{
    my $patches = shift;
    
    # info about patches from the developer point of view
    # it is a hash, the key is the developer name, the value is:
    #	  a hash, keys introduce perl-like structure items:
    #	      'num_patches' 	 	... integer, total number of patches per developer
    #	      'num_patches_no_issue' 	... integer, number of patches without any issue number per developer
    #	      'targets' 		... a hash, the key is the target (section) name, the is value:
    #	          a hash, keys introduce perl-like structure items:
    #		      'num_patches' 	 	... integer, total number of patches per developer and target
    #                 'num_patches_no_issue' 	... integer, number of patches without any issue number per developer and target
    my %developers = ();

    # find patches without any assigned issue number
    foreach my $patch (keys %{$patches}) {
    
	next if ($patch =~ /cws\-/);
    
	my $developer = $patches->{$patch}->{'developer'};
    	unless (defined $developers{$developer}) {
	    # new entry for a new developer
	    $developers{$developer} = {};
	    $developers{$developer}{'num_patches'} = 0;
	    $developers{$developer}{'num_patches_no_issue'} = 0;
	    $developers{$developer}{'targets'} = ();
	}

	# count the patch only once even if it is used in more targets,
	# so take only the first target
	my $target = $patches->{$patch}->{'targets'}->[0];
    	unless ($developers{$developer}{'targets'}{$target}) {
	    # new entry for a new target (section) used by the given developer
	    $developers{$developer}{'targets'}{$target} = {};
	    $developers{$developer}{'targets'}{$target}{'num_patches'} = 0;
	    $developers{$developer}{'targets'}{$target}{'num_patches_no_issue'} = 0;
	}

	# total number of patches for the given developer and target
	++$developers{$developer}{'num_patches'};
	++$developers{$developer}{'targets'}{$target}{'num_patches'};
	
	# number of patches without any assigned issue for the given developer and target
	unless ($patches->{$patch}->{'issue'}) {
	    ++$developers{$developer}{'num_patches_no_issue'};
	    ++$developers{$developer}{'targets'}{$target}{'num_patches_no_issue'};
	}
    }

    # it is an array sorted by total numbers per developer, the value is:
    #     a hash, keys introduce perl-like structure items:
    #         'developer' ... string, developer name
    #         'targets'   ... an array, values are sorted target names
    my @developers_order = ();
    # sort by total numbers per developer
    foreach my $developer (sort { $developers{$b}{'num_patches_no_issue'}/$developers{$b}{'num_patches'} <=> $developers{$a}{'num_patches_no_issue'}/$developers{$a}{'num_patches'}
	                          ||
				  $developers{$b}{'num_patches_no_issue'} <=> $developers{$a}{'num_patches_no_issue'}
				} keys %developers) {
	my %developer_targets = ();
	$developer_targets{'developer'} = $developer;
	$developer_targets{'targets'} = [];
	# sort by the numbers per target for each developer
	foreach my $target (sort { $developers{$developer}{'targets'}{$b}{'num_patches_no_issue'}/$developers{$developer}{'targets'}{$b}{'num_patches'} <=> $developers{$developer}{'targets'}{$a}{'num_patches_no_issue'}/$developers{$developer}{'targets'}{$a}{'num_patches'}
			           ||
				   $developers{$developer}{'targets'}{$b}{'num_patches_no_issue'} <=> $developers{$developer}{'targets'}{$a}{'num_patches_no_issue'}
	                         } keys % {$developers{$developer}{'targets'}}) {
	    push @{$developer_targets{'targets'}}, $target;
	}
#	print "Targets sorted $developer_targets{'developer'}: @{$developer_targets{'targets'}}\n";
	push @developers_order, \%developer_targets;
    }

    print "Number of patches without assigned an issue number per developer\n";
    print "================================================================\n";
    foreach my $developer_targets (@developers_order) {
	my $developer = $developer_targets->{'developer'};
	# skip developes with all patches reported
	next unless ($developers{$developer}{'num_patches_no_issue'});
	print "$developer($developers{$developer}{'num_patches_no_issue'}/$developers{$developer}{'num_patches'}):";
	my $target_delimiter = '';
	foreach my $target (@ {$developer_targets->{'targets'}}) {
	    # skip targets with all patches reported
	    next unless ($developers{$developer}{'targets'}{$target}{'num_patches_no_issue'});
	    print "$target_delimiter $target($developers{$developer}{'targets'}{$target}{'num_patches_no_issue'}/$developers{$developer}{'targets'}{$target}{'num_patches'})";
	    $target_delimiter = ',';
	}
	print "\n";
	print "---\n";
    }
}

(@ARGV > 1) ||
    die "Syntax:\n".
    "apply <path-to-patchdir> <src root> --tag=<src680-m90> [--distro=Debian [--distro=Binfilter [...]]] [--quiet] [--dry-run] [ patch flags ]\n" .
    "apply <path-to-patchdir> --series-to\n" .
    "apply <path-to-patchdir> --add-developer\n" .
    "apply <path-to-patchdir> --find-unused\n" .
    "apply <path-to-patchdir> --statistic-no-issue\n";

%options = ();

$quiet = 0;
$remove = 0;
$export = 0;
$opts = "";
@distros = ();
$tag = '';
$dry_run = 0;
$find_unused = 0;
$add_developer = 0;
$statistic = 0;
$statistic_no_issue = 0;
@required_opts = ( 'PATCHPATH', 'OLDEST_SUPPORTED' );
@arguments = ();

# This hash includes information about all patch entries
# The key is the patch name, the value is:
#     a pointer to a hash, the key is the target name where the patch belongs, the value is:
#         a pointer to a hash, keys introduce perl-like structure items:
#	      'issue'     ... string, the issue number that is connected with the patch
#	      'developer' ... string, the developer the is responsible for the patch
#	      'targets'   ... a pointer to an array, the values are names of targets where the patch is used
%unfiltered_patch_list = ();


foreach $a (@ARGV) {
	if ($a eq '-R') {
	    $remove = 1;	    
	    push @arguments, $a;
	} elsif ($a =~ m/--series-from=(.*)/) {
	    $export = 1;
	    $quiet = 1;
	    push @distros, $1;
	} elsif ($a eq '--quiet') {
	    $quiet = 1;
	} elsif ($a =~ m/--distro=(.*)/) {
	    push @distros, $1;
	} elsif ($a =~ m/--add-developer/) {
	    $add_developer = 1;
	    $quiet = 1;
	} elsif ($a =~ m/--find-unused/) {
	    $find_unused = 1;
	} elsif ($a =~ m/--statistic-no-issue/) {
	    $statistic = 1;
	    $statistic_no_issue = 1;
	    $quiet = 1;
	} elsif ($a =~ m/--tag=(.*)/) {
	    $tag = $1;
	} elsif ($a =~ m/--dry-run/g) {
	    $dry_run = 1;
	} else {
	    push @arguments, $a;
	}
}

if ( ! @distros ) {
    @distros = ( "Debian" );
}

$patch_dir = shift (@arguments);
substr ($patch_dir, 0, 1) eq '/' || die "apply.pl requires absolute paths";

$apply_list = $patch_dir.'/apply';

print "Execute with $opts for distro(s) '" . join( " ", @distros ) . "'\n" unless ($quiet);

if ($dry_run || $add_developer || $find_unused || $statistic) {
    $tag = '' if ($find_unused);

    my @Patches = list_patches (@distros);

    if ($add_developer) {
	add_developer_info();
    } elsif ($find_unused) {
	check_for_unused (\%unfiltered_patch_list);
    } elsif ($statistic) {
	if ($statistic_no_issue) {
	    print_statistic_no_issue (\%unfiltered_patch_list);
	}
    } else {
	printf "Dry-run: exiting before applying patches\n";
    }
} elsif ($export) {
    export_series();

} else {
    $dest_dir = shift (@arguments);
    substr ($dest_dir, 0, 1) eq '/' || die "apply.pl requires absolute paths";
    $applied_patches = $dest_dir.'/applied_patches';

    $opts = join ' ', @arguments;
    $base_cmd = "patch -l -p0 $opts -d $dest_dir";
    if (is_old_patch_version()) {
	$base_cmd = 'sed \'s/^\(@.*\)\r$/\1/\' | ' . $base_cmd;
    }

    if ($remove) {
	remove_patches();
    } else {
	apply_patches();
    }
}
