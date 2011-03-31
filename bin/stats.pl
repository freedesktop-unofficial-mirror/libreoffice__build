#!/usr/bin/perl -w

# segment by Date, then by Product, then count
my %data;
my %products;

# Analysing stats:
#
# grep for 'multi' - yields the Windows installer ... (also grep for 'all_lang') - all of them [!]
# grep for 'Linux' and 'en-US' yields total Linux main binary downloads ...
# grep for 'Mac' and 'en-US' yields total Mac main binary numbers ...

while (<>) {
    chomp();
    my $line = $_;
#    print "line '$_'\n";
#    17424    2011-01-25      LibO	Win-x86	3.3.0	        all_lang	qa	1
    if ($line =~ m/^\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*$/) {
	my ($id, $date, $product, $osname, $version, $lang, $country, $count) = ($1, $2, $3, $4, $5, $6, $7, $8);
#	print "$count downloads on date $date, os $osname $lang\n";

	if ($date lt '2011-01-25') {
#	    print STDERR "ignoring $date\n";
	    next
	}

	my $clean_product;

	# Ignore Mac / Linux help packs etc.
	if (($osname =~ /Linux/ || $osname =~ /MacOS/) && $lang eq 'en-US') {
	    $clean_product = $osname;
	}
	# Detect Windows distinctions
 	if (($product eq 'LibO' || $product eq 'LO') &&
	    $osname =~ /Win/ && ($lang =~ /multi/ || $lang =~ /all_lang/)) {
	    $clean_product = "$osname-$lang";
	}

	# Detect PortableOffice distinctions
	if ($product eq 'LibreOfficePortable') {
	    $clean_product = $product;
	}

	if (!defined $clean_product) {
#	    print "uninteresting line '$line'\n";
	    next;
	}

	$products{$clean_product} = 1;
	if (!defined $data{$date}) {
	    my %byproduct;
	    $data{$date} = \%byproduct;
	}
	if (!defined ($data{$date}->{$clean_product})) {
	    $data{$date}->{$clean_product} = 0;
	}
	$data{$date}->{$clean_product} += $count;
# 	print "count for '$date' and '$clean_product' == $data{$date}->{$clean_product} [ added $count ]\n";
    } else {
#	print STDERR "malformed line '$_'\n";
    }
}

# now output this as a spreadsheet ... fods ...
print << "EOF"
<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
                 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
                 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
                 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
                 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
                 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
                 xmlns:xlink="http://www.w3.org/1999/xlink"
                 xmlns:dc="http://purl.org/dc/elements/1.1/"
                 xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0"
                 xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"
                 xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"
                 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
                 xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0"
                 xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0"
                 xmlns:math="http://www.w3.org/1998/Math/MathML"
                 xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0"
                 xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0"
                 xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0"
                 xmlns:ooo="http://openoffice.org/2004/office"
                 xmlns:ooow="http://openoffice.org/2004/writer"
                 xmlns:oooc="http://openoffice.org/2004/calc"
                 xmlns:dom="http://www.w3.org/2001/xml-events"
                 xmlns:xforms="http://www.w3.org/2002/xforms"
                 xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                 xmlns:rpt="http://openoffice.org/2005/report"
                 xmlns:of="urn:oasis:names:tc:opendocument:xmlns:of:1.2"
                 xmlns:xhtml="http://www.w3.org/1999/xhtml"
                 xmlns:grddl="http://www.w3.org/2003/g/data-view#"
                 xmlns:tableooo="http://openoffice.org/2009/table"
                 xmlns:field="urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0"
                 xmlns:formx="urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0"
                 xmlns:css3t="http://www.w3.org/TR/css3-text/"
                 office:version="1.2"
                 grddl:transformation="http://docs.oasis-open.org/office/1.2/xslt/odf2rdf.xsl"
                 office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
   <office:styles>
      <number:date-style style:name="isodatenum">
         <number:year number:style="long"/>
         <number:text>-</number:text>
         <number:month number:style="long"/>
         <number:text>-</number:text>
         <number:day number:style="long"/>
      </number:date-style>
      <style:style style:name="boldheader" style:family="table-cell" style:parent-style-name="Default">
         <style:text-properties fo:font-style="italic" fo:font-weight="bold"/>
      </style:style>
      <style:style style:name="isodate" style:family="table-cell" style:parent-style-name="Default" style:data-style-name="isodatenum"/>
   </office:styles>
   <office:body>
      <office:spreadsheet>
         <table:table table:name="RawData">
            <table:table-row>
               <table:table-cell table:style-name="boldheader" office:value-type="string">
                  <text:p>Date</text:p>
               </table:table-cell>
EOF
;
@prods = sort keys %products;
for my $product (@prods) {
print << "EOF"
               <table:table-cell table:style-name="boldheader" office:value-type="string">
                  <text:p>$product</text:p>
               </table:table-cell>
EOF
	    ;
}
print << "EOF"
               <table:table-cell table:style-name="boldheader" office:value-type="string">
                  <text:p>Total</text:p>
               </table:table-cell>
            </table:table-row>
EOF
;

my $row = 1;

# FIXME: ODF is -incredibly- lame in this regard ... we badly want R1C1 style referencing here [!]
my $colcount = @prods;
die ("odff needs R1C1") if ($colcount > 25);
my $colname = chr (ord('A') + $colcount);
print STDERR "cols: $colcount - colname $colname @prods\n";

for my $date (sort keys %data) {
print << "EOF"
            <table:table-row>
               <table:table-cell table:style-name="isodate" office:value-type="date" office:date-value="$date"/>
EOF
;
    for my $product (@prods) {
	my $count = $data{$date}->{$product};
	$count = 0 if (!defined $count);
print << "EOF"
               <table:table-cell office:value-type="float" office:value="$count"/>
EOF
;
    }
    $row++;
print << "EOF"
               <table:table-cell table:formula="of:=SUM([.B$row:.$colname$row])" office:value-type="float"/>
            </table:table-row>
EOF
;
}

print << "EOF"
         </table:table>
      </office:spreadsheet>
   </office:body>
</office:document>
EOF
;
