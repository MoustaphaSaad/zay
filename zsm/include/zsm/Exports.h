
#ifndef ZSM_EXPORT_H
#define ZSM_EXPORT_H

#ifdef ZSM_STATIC_DEFINE
#  define ZSM_EXPORT
#  define ZSM_NO_EXPORT
#else
#  ifndef ZSM_EXPORT
#    ifdef zsm_EXPORTS
        /* We are building this library */
#      define ZSM_EXPORT 
#    else
        /* We are using this library */
#      define ZSM_EXPORT 
#    endif
#  endif

#  ifndef ZSM_NO_EXPORT
#    define ZSM_NO_EXPORT 
#  endif
#endif

#ifndef ZSM_DEPRECATED
#  define ZSM_DEPRECATED __declspec(deprecated)
#endif

#ifndef ZSM_DEPRECATED_EXPORT
#  define ZSM_DEPRECATED_EXPORT ZSM_EXPORT ZSM_DEPRECATED
#endif

#ifndef ZSM_DEPRECATED_NO_EXPORT
#  define ZSM_DEPRECATED_NO_EXPORT ZSM_NO_EXPORT ZSM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ZSM_NO_DEPRECATED
#    define ZSM_NO_DEPRECATED
#  endif
#endif

#endif /* ZSM_EXPORT_H */
