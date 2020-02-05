#
# meataxe64: meataxe64
#
# This file contains package meta data. For additional information on
# the meaning and correct usage of these fields, please consult the
# manual of the "Example" package as well as the comments in its
# PackageInfo.g file.
#
SetPackageInfo( rec(

PackageName := "meataxe64",
Subtitle := "meataxe64",
Version := "0.1",
Date := "23/08/2019", # dd/mm/yyyy format
License := "GPL-2.0-or-later",

Persons := [
  rec(
    IsAuthor := true,
    IsMaintainer := true,
    FirstNames := "Steve",
    LastName := "Linton",
    WWWHome := "https://www.cs.st-andrews.ac.uk/directory/person?id=sal",
    Email := "steve.linton@st-andrews.ac.uk",
    PostalAddress := "School of Computer Science\nJack Cole Building\nNorth Haugh\nSt Andrews\nFife\nKY16 9SS\nScotland",
    Place := "St Andrews",
    Institution := "University of St Andrews",
  ),
  rec(
    IsAuthor := true,
    IsMaintainer := false,
    FirstNames := "Richard",
    LastName := "Parker",
    WWWHome := "http://meataxe64.wordpress.com",
    Email := "richpark54@hotmail.co.uk",
    PostalAddress := "TODO",
    Place := "Cambridge",
    Institution := "TODO",
  ),
  rec(
    IsAuthor := true,
    IsMaintainer := true,
    FirstNames := "Markus",
    LastName := "Pfeiffer",
    WWWHome := "http://www.morphism.de/~markusp/",
    Email := "markus.pfeiffer@st-andrews.ac.uk",
    PostalAddress := "School of Computer Science\nJack Cole Building\nNorth Haugh\nSt Andrews\nFife\nKY16 9SS\nScotland",
    Place := "St Andrews",
    Institution := "University of St Andrews",
  ),
],

SourceRepository := rec(
    Type := "git",
    URL := Concatenation( "https://github.com/gap-packages/", ~.PackageName ),
),
IssueTrackerURL := Concatenation( ~.SourceRepository.URL, "/issues" ),
#SupportEmail   := "TODO",
PackageWWWHome  := "https://gap-packages.github.io/meataxe64/",
PackageInfoURL  := Concatenation( ~.PackageWWWHome, "PackageInfo.g" ),
README_URL      := Concatenation( ~.PackageWWWHome, "README.md" ),
ArchiveURL      := Concatenation( ~.SourceRepository.URL,
                                 "/releases/download/v", ~.Version,
                                 "/", ~.PackageName, "-", ~.Version ),

ArchiveFormats := ".tar.gz",

##  Status information. Currently the following cases are recognized:
##    "accepted"      for successfully refereed packages
##    "submitted"     for packages submitted for the refereeing
##    "deposited"     for packages for which the GAP developers agreed
##                    to distribute them with the core GAP system
##    "dev"           for development versions of packages
##    "other"         for all other packages
##
Status := "dev",

AbstractHTML   :=  "",

PackageDoc := rec(
  BookName  := "meataxe64",
  ArchiveURLSubset := ["doc"],
  HTMLStart := "doc/chap0.html",
  PDFFile   := "doc/manual.pdf",
  SixFile   := "doc/manual.six",
  LongTitle := "meataxe64",
),

Dependencies := rec(
  GAP := ">= 4.11",
                   NeededOtherPackages := [ [ "GAPDoc", ">= 1.5" ], 
                           [ "AutoDoc", ">= 0.0.0"],
                          [ "datastructures", ">= 0.0.0"] ],
  SuggestedOtherPackages := [ ],
  ExternalConditions := [ ],
),

AvailabilityTest := function()
        return true;
    end,

TestFile := "tst/testinstall.g",

#Keywords := [ "TODO" ],

));


