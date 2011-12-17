#if ! defined HASH_SET_INCLUDER_H
#define HASH_SET_INCLUDER_H

#if defined NO_HASH_THINGS

#include <set>
#define HASH_SET std::set
#define HASH_COMPARE error_no_such_entity

#else

#if defined _MSC_VER

// 2008/08/13 Modified to use TR1 container, if VC++ version is 9 or later

#if _MSC_VER >= 1500

#include <unordered_set>
#define HASH_SET std::tr1::unordered_set

#else

#include <hash_set>
#define HASH_SET stdext::hash_set

#endif

#elif defined __GNUC__

// 2008/06/25 Modified to use TR1 container.

#include <tr1/unordered_set>
#define HASH_SET std::tr1::unordered_set

#endif

#endif

#endif // HASH_SET_INCLUDER_H
