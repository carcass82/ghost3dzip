#pragma once

inline
unsigned log2upper(register unsigned n)
{
    --n, n <<= 1;
    unsigned ret = 0;
    while ((n >> ret) > 1)
	++ret;
    return ret;
};
