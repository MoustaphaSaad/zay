
#ifndef FLAG_EXPORT_H
#define FLAG_EXPORT_H

#ifdef FLAG_STATIC_DEFINE
#  define FLAG_EXPORT
#  define FLAG_NO_EXPORT
#else
#  ifndef FLAG_EXPORT
#    ifdef flag_EXPORTS
        /* We are building this library */
#      define FLAG_EXPORT 
#    else
        /* We are using this library */
#      define FLAG_EXPORT 
#    endif
#  endif

#  ifndef FLAG_NO_EXPORT
#    define FLAG_NO_EXPORT 
#  endif
#endif

#ifndef FLAG_DEPRECATED
#  define FLAG_DEPRECATED __declspec(deprecated)
#endif

#ifndef FLAG_DEPRECATED_EXPORT
#  define FLAG_DEPRECATED_EXPORT FLAG_EXPORT FLAG_DEPRECATED
#endif

#ifndef FLAG_DEPRECATED_NO_EXPORT
#  define FLAG_DEPRECATED_NO_EXPORT FLAG_NO_EXPORT FLAG_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FLAG_NO_DEPRECATED
#    define FLAG_NO_DEPRECATED
#  endif
#endif

#endif /* FLAG_EXPORT_H */
