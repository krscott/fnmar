# krslib

An experimental C util library.

This library makes heavy use of x-macros in order to implement generic code
which in a typical production environment would be auto-generated in a pre-build
step. This is a concious decision to add macro magic in order to minimize build
complexity.

i.e. I just want to see how much of Zig comptime or Rust traits I can get in 
pure C*

\*codegen using posix-standard tools allowed for quality of life
