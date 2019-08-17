
#ifndef ALP_EXPORT_H
#define ALP_EXPORT_H

#ifdef ALP_STATIC_DEFINE
#  define ALP_EXPORT
#  define ALP_NO_EXPORT
#else
#  ifndef ALP_EXPORT
#    ifdef alp_EXPORTS
        /* We are building this library */
#      define ALP_EXPORT 
#    else
        /* We are using this library */
#      define ALP_EXPORT 
#    endif
#  endif

#  ifndef ALP_NO_EXPORT
#    define ALP_NO_EXPORT 
#  endif
#endif

#ifndef ALP_DEPRECATED
#  define ALP_DEPRECATED __declspec(deprecated)
#endif

#ifndef ALP_DEPRECATED_EXPORT
#  define ALP_DEPRECATED_EXPORT ALP_EXPORT ALP_DEPRECATED
#endif

#ifndef ALP_DEPRECATED_NO_EXPORT
#  define ALP_DEPRECATED_NO_EXPORT ALP_NO_EXPORT ALP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ALP_NO_DEPRECATED
#    define ALP_NO_DEPRECATED
#  endif
#endif

#endif /* ALP_EXPORT_H */
