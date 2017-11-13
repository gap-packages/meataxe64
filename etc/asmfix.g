#
# Function to do minimal conversion of Richard's Linux .s files
# so they will build on OS X. Deletes .size and .type and adds _ before globals
#
#
OSXifyAsmFile := function(infile, outfile)
    local  getDirective, recordGlobal, getLabel, s, lines, globs, 
           olines, l, res, glob;
    
    getDirective := function(line)
        local  i, dir;
        i := 1;
        while i <= Length(l) and l[i] in WHITESPACE do
            i := i+1;
        od;
        if i > Length(l) or l[i] <> '.' then
            return false;
        fi;
        i := i+1;
        dir := "";        
        while i <= Length(l) and (l[i] in LETTERS or l[i] in DIGITS) do
            Add(dir,l[i]);
            i := i+1;
        od;
        return [dir,l{[i..Length(l)]}];
    end;
    
    recordGlobal := function(rest)
        local g;
        g := NormalizedWhitespace(rest);
        Add(globs, g);
        return g;
    end;
    
    getLabel := function(line)
        local  i, g;
        i := 1;
        g := "";        
        while i <= Length(l) and l[i] in WHITESPACE do
            i := i+1;
        od;
        while i <= Length(l) and (l[i] in LETTERS or l[i] in DIGITS) do
            Add(g,l[i]);
            i := i+1;
        od;
        if i <= Length(l) and l[i] = ':' then
            return g;
        else
            return false;
        fi;
    end;
    
            
    
    s := StringFile(infile);
    lines := SplitString(s, "\n");
    globs := [];    
    olines := [];    
    for l in lines do
        res := getDirective(l);        
        if false <> res then
            if res[1] = "globl" then
                glob := recordGlobal(res[2]);
                Add(olines, ReplacedString(l, glob, Concatenation("_",glob)));                
            elif res[1] <> "type" and res[1] <> "size" then
                Add(olines,l);
            fi;
            continue;                
        fi;
        res := getLabel(l);
        if res in globs then
            Add(olines, ReplacedString(l, res, Concatenation("_", res)));
        else
            Add(olines,l);            
        fi;
    od;
    FileString(outfile, JoinStringsWithSeparator(olines,"\n"));
end;

            
        
           
        
