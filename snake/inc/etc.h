#ifndef ETC_H
#define ETC_H

// wraps around the macro code to make ending ; necessary
#define DO(...) do{__VA_ARGS__}while(0)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PI 3.14159265358979323846

#endif // ETC_H