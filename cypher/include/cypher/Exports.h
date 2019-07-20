
#ifndef CYPHER_EXPORT_H
#define CYPHER_EXPORT_H

#ifdef CYPHER_STATIC_DEFINE
#  define CYPHER_EXPORT
#  define CYPHER_NO_EXPORT
#else
#  ifndef CYPHER_EXPORT
#    ifdef cypher_EXPORTS
        /* We are building this library */
#      define CYPHER_EXPORT 
#    else
        /* We are using this library */
#      define CYPHER_EXPORT 
#    endif
#  endif

#  ifndef CYPHER_NO_EXPORT
#    define CYPHER_NO_EXPORT 
#  endif
#endif

#ifndef CYPHER_DEPRECATED
#  define CYPHER_DEPRECATED __declspec(deprecated)
#endif

#ifndef CYPHER_DEPRECATED_EXPORT
#  define CYPHER_DEPRECATED_EXPORT CYPHER_EXPORT CYPHER_DEPRECATED
#endif

#ifndef CYPHER_DEPRECATED_NO_EXPORT
#  define CYPHER_DEPRECATED_NO_EXPORT CYPHER_NO_EXPORT CYPHER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CYPHER_NO_DEPRECATED
#    define CYPHER_NO_DEPRECATED
#  endif
#endif

#endif /* CYPHER_EXPORT_H */
