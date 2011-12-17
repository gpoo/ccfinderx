#if ! defined HASH_MAP_INCLUDER_H
#define HASH_MAP_INCLUDER_H

#if defined NO_HASH_THINGS

#include <map>
#define HASH_MAP std::map
#define HASH_COMPARE error_no_such_entity

#else

#if defined _MSC_VER

// 2008/08/13 Modified to use TR1 container, if VC++ version is 9 or later

#if _MSC_VER >= 1500

#include <unordered_map>
#define HASH_MAP std::tr1::unordered_map
#define HASH_COMPARE std::tr1::hash

#else

#include <hash_map>
#define HASH_MAP stdext::hash_map
#define HASH_COMPARE stdext::hash_compare

#endif

#elif defined __GNUC__

// 2008/06/25 Modified to use TR1 container.

#include <tr1/unordered_map>
#define HASH_MAP std::tr1::unordered_map
#define HASH_COMPARE std::tr1::hash

#endif

#endif

#endif // HASH_MAP_INCLUDER_H
