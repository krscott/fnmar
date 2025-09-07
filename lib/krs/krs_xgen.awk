BEGIN {
    IDLE = 0
    FIND_DEF = 1
    CURLY = 2
    FIELDS = 3

    state = IDLE
}

FNR==1 {
    fname = FILENAME
    sub(".*/", "", fname)  # remove Unix-style path
    sub(".*\\\\", "", fname)  # remove Windows-style path

    fname_ident = fname
    sub("\\.[^.]*$", "", fname_ident)  # remove extension
    gsub(/[^[:alnum:]]+/, "_", fname_ident)
    sub(/^_+/, "", fname_ident)
    sub(/_+$/, "", fname_ident)
    fname_ident = toupper(fname_ident)
    print "#ifndef " fname_ident "_XGEN_H_"
    print "#define " fname_ident "_XGEN_H_"
    print ""
    print "/* Generated based on " fname " */"
    print ""
}

END {
    print "#endif"
}

state == IDLE && /\/\* #xgen \*\// {
    state = FIND_DEF
    structure = ""
    name = ""
}

state == FIND_DEF && match($0, /^(typedef[[:space:]]+)?(enum|struct)\s+([[:alnum:]_]+)/, m) {
    state = CURLY
    structure = m[2]
    name = m[3]
    field_idx = 0
}

state != IDLE {print "// " $0}

state == CURLY && /{/ { state = FIELDS }

state == FIELDS && structure == "enum" && match($0, /^[[:space:]]*([[:alnum:]_]+)([[:space:]=].*)?,$/, m) {
    field_name[field_idx++] = m[1]
    next
}
state == FIELDS && structure == "struct" && match($0, /^[[:space:]]*(.*[^[:alnum:]_])([[:alnum:]_]+)(\[(.*)\])?;$/, m) {
    if (match(m[1], /^(struct|enum)[[:space:]]+([[:alnum:]_]+)[[:space:]]*$/, m_complex)) {
        field_is_struct[field_idx] = m_complex[1] == "struct"
        field_is_enum[field_idx] = m_complex[1] == "enum"
        field_type[field_idx] = m_complex[2]
    } else {
        field_is_struct[field_idx] = 0
        field_is_enum[field_idx] = 0
        field_type[field_idx] = m[1]
    }
    field_name[field_idx] = m[2]
    field_is_array[field_idx] = m[3] != ""
    field_array_len[field_idx] = m[4]

    ++field_idx
    next
}

state == FIELDS && /;/ {
    if (structure == "struct") {
        print "#define xgen_" name "(F) \\"
        for (i = 0; i < field_idx; i++) {
            printf "    F("
            if (field_is_struct[i] && field_is_array[i]) {
                printf "xf_struct_array, " field_type[i] ", " field_name[i] ", " field_array_len[i]
            } else if (field_is_struct[i]) {
                printf "xf_struct, " field_type[i] ", " field_name[i]
            } else if (field_is_enum[i] && field_is_array[i]) {
                printf "xf_enum_array, " field_type[i] ", " field_name[i] ", " field_array_len[i]
            } else if (field_is_enum[i]) {
                printf "xf_enum, " field_type[i] ", " field_name[i]
            } else if (field_is_array[i]) {
                printf "xf_simple_array, " field_type[i] ", " field_name[i] ", " field_array_len[i]
            } else {
                printf "xf_simple, " field_type[i] ", " field_name[i]
            }
            printf ")"
            if (i < field_idx - 1) {
                print " \\"
            }
        }
    } else if (structure == "enum") {
        print "#define xgen_" name "(X) \\"
        for (i = 0; i < field_idx; i++) {
            printf "    X(" field_name[i] ")"
            if (i < field_idx - 1) {
                print " \\"
            }
        }
    }
    print ""
    print ""
    state = IDLE
}
