#ifndef UTILS_H
#define UTILS_H

#include <string>

static inline void bzero(void *dst, size_t sz)
{
    int CharElements = sz & (sizeof(size_t) - 1);
    sz -= CharElements;

    char *p1 = (char *)dst;
    size_t *p2 =  (size_t *)((char *)dst + CharElements);

    while(CharElements--)
    {
        *p1++ = 0;
    }

    while(sz)
    {
        *p2++ = 0;
        sz -= sizeof(size_t);
    }
}

static inline std::string replace_all(std::string str, const std::string &from, const std::string &to)
{
    for(size_t start_pos = str.find(from, 0); start_pos != std::string::npos; start_pos = str.find(from, start_pos))
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}

#endif