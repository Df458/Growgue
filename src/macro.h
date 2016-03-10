#ifndef MACRO_H
#define MACRO_H

#define macro_str(m) name_str(m)
#define name_str(m) #m
#define clamp(x, lo, hi) (x >= lo ? (x <= hi ? x : hi) : lo)

#endif
