# A simple awk script to help migrating patches in the master
# Copyright LGPLv3 (or later) / MPL 2011 Canonical Ltd.

BEGIN {
	state="presection"
	FS=","
	patchpattern = "/" PATCHNAME "/"
	comment = ""
	lastcomment = ""
	sectioncomment = ""
}

/\[.*\]/ {
	section = $0
	sub(/^\[ */,"",section)
	sub(/ *]$/,"",section)
	state = "sectionheader"
	sectioncomment = ""
}

{
	if ($0 ~ /^#/)
	{
		commentline = $0
		sub(/^# */,"",commentline)
		if (state == "sectionheader")
		{
			sectioncomment = sectioncomment "\n  " commentline
		}
		else if(state == "sectionbody")
		{
			comment = comment "\n  " commentline
		}
	}
	else if ($1 ~ PATCHNAME)
	{
		issues = ""
		author = ""
		for (i = 2; i <= NF; i++)
		{
			if ($i ~ /#/)
			{
				issues = issues "," $i
			}
			else
			{
				author = author "," $i
			}
		}
		sub(/^, */, "", issues)
		sub(/^, */, "", author)
		if (comment=="")
		{
			comment = lastcomment
			shortmessage = "apply build repository patch " PATCHNAME
		}
		else
		{
			shortmessage = comment
			gsub(/\n/, " ", shortmessage)
			gsub(/  /, " ", shortmessage)
			gsub(/  /, " ", shortmessage)
			shortmessage = substr(shortmessage, 0, 70)
		}
		if (issues !="")
			shortmessage = issues ": " shortmessage
		print "#!/bin/bash"
		print "[ -n ${SOLARSRC} ] || (echo \"no environment set!\" >2 && false)"
		print "cd ${SOLARSRC}"
		print "MESSAGEFILE=`mktemp`"
		print "cat > ${MESSAGEFILE} <<\"ENDOFAUTOMATEDLOGMESSAGE\""
		print shortmessage
		print ""
		print "information from the apply file:"
		print " patchname: " PATCHNAME
		print " issues: " issues
		print " author: " author
		print " section: " section
		print " comment: " comment
		print " sectioncomment: " sectioncomment
		print ""
		print "git log from build repository:"
		gitlog = "git log " PATCHNAME
		while ((gitlog | getline) > 0)
		{
			print $0
		}
		close(gitlog)
		print "ENDOFAUTOMATEDLOGMESSAGE"
		print ""
		print "patch -p0 <<\"ENDOFMIGRATEDPATCH\""
		patchcat = "cat " PATCHNAME
		while ((patchcat | getline) > 0)
		{
			print $0
		}
		print "ENDOFMIGRATEDPATCH"
		print ""
		print "./g add -A"
		if (author != "")
			author = " --author='" author "'"
		# we need to skip hooks because otherwise git logs cannot go in the message file
		print "./g commit -F ${MESSAGEFILE} --no-verify" author
		print "rm ${MESSAGEFILE}"
		state = "sectionbody"
			
	}
	else
	{
		lastcomment = comment
		comment = ""
		state = "sectionbody"
	}
}
