BEGIN {
    IDLE = 0
    STRUCT = 1
    CURLY = 2
    FIELDS = 3

    state = IDLE
}

FNR==1 {
    fname = FILENAME
    sub(".*/", "", fname)  # remove Unix-style path
    sub(".*\\\\", "", fname)  # remove Windows-style path

    header_guard = fname
    sub("\\.[^.]*$", "", header_guard)  # remove extension
    gsub(/[^[:alnum:]]+/, "_", header_guard)
    sub(/^_+/, "", header_guard)
    sub(/_+$/, "", header_guard)
    header_guard = toupper(header_guard)
    print "#ifndef XGEN_" header_guard "_H_"
    print "#define XGEN_" header_guard "_H_"
    print ""
    print "/* Generated based on " fname " */"
    print ""
}

END {
    print "#endif"
}

state == IDLE && /\/\* #xgen \*\// {
    state = STRUCT
    structure = ""
    name = ""
    field_idx = 0
}

state == STRUCT && /^struct|enum|union \w+/ {
    state = CURLY
    structure = $1
    name = $2
}

state == CURLY && /{/ { state = FIELDS }

state == FIELDS && structure == "enum" && match($0, /(\w+),/, m) {
    field_names[field_idx++] = m[1]
    next
}

state == FIELDS && /;/ {
    print "#define xgen_" name "(X) \\"
    for (i = 0; i < field_idx; i++) {
        printf "    X(" field_names[i] ")"
        if (i < field_idx - 1) {
            print " \\"
        }
    }
    print ""
    print ""
    state = IDLE
}
