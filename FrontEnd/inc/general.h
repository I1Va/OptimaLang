#ifndef GENERAL_H
#define GENERAL_H

#include <string.h>

const size_t date_nmemb = 16;
const size_t time_nmemb = 16;
const size_t MINI_BUFER_SZ = 16;
const size_t MEDIUM_BUFER_SZ = 128;
const size_t STR_F_BORDER_SZ = 100;

#define GRN "\e[0;32m"
#define WHT "\e[0;20m"
#define RED "\e[0;31m"
#define YEL "\e[0;33m"

#define HTML_GRN "<font color=\"green\">"
#define HTML_BLACK "<font color=\"black\">"
#define HTML_RED "<font color=\"red\">"
#define HTML_YEL "<font color=\"yellow\">"

#define printf_red(str_, ...) printf(RED str_ WHT, ##__VA_ARGS__)
#define printf_wht(str_, ...) printf(WHT str_ WHT, ##__VA_ARGS__)
#define printf_grn(str_, ...) printf(GRN str_ WHT, ##__VA_ARGS__)
#define printf_yel(str_, ...) printf(YEL str_ WHT, ##__VA_ARGS__)

#define fprintf_red(stream, str_, ...) fprintf(stream, RED str_ WHT, ##__VA_ARGS__)
#define fprintf_wht(stream, str_, ...) fprintf(stream, WHT str_ WHT, ##__VA_ARGS__)
#define fprintf_grn(stream, str_, ...) fprintf(stream, GRN str_ WHT, ##__VA_ARGS__)
#define fprintf_yel(stream, str_, ...) fprintf(stream, YEL str_ WHT, ##__VA_ARGS__)

#define fprintf_html_red(stream, str_, ...) fprintf(stream, HTML_RED str_, ##__VA_ARGS__)
#define fprintf_html_grn(stream, str_, ...) fprintf(stream, HTML_GRN str_, ##__VA_ARGS__)
#define fprintf_html_black(stream, str_, ...) fprintf(stream, HTML_BLACK str_, ##__VA_ARGS__)
#define fprintf_html_yel(stream, str_, ...) fprintf(stream, HTML_YEL str_, ##__VA_ARGS__)

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define CLEAR_MEMORY(mark) goto mark;

#define FREE(ptr)              \
    do {                       \
        free(ptr); ptr = NULL; \
    } while(0);

#ifdef _DEBUG
    #define debug(str_, ...) fprintf_red(stderr, "{%s} [%s: %d]: descr{" str_ "}\n", __FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
    #define ON_DEBUG(...) __VA_ARGS__
#else
    #define debug(str_, ...) ;
    #define ON_DEBUG(...)
#endif // _DEBUG


#endif // GENERAL_H