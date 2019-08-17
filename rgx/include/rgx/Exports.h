
#ifndef RGX_EXPORT_H
#define RGX_EXPORT_H

#ifdef RGX_STATIC_DEFINE
#  define RGX_EXPORT
#  define RGX_NO_EXPORT
#else
#  ifndef RGX_EXPORT
#    ifdef rgx_EXPORTS
        /* We are building this library */
#      define RGX_EXPORT 
#    else
        /* We are using this library */
#      define RGX_EXPORT 
#    endif
#  endif

#  ifndef RGX_NO_EXPORT
#    define RGX_NO_EXPORT 
#  endif
#endif

#ifndef RGX_DEPRECATED
#  define RGX_DEPRECATED __declspec(deprecated)
#endif

#ifndef RGX_DEPRECATED_EXPORT
#  define RGX_DEPRECATED_EXPORT RGX_EXPORT RGX_DEPRECATED
#endif

#ifndef RGX_DEPRECATED_NO_EXPORT
#  define RGX_DEPRECATED_NO_EXPORT RGX_NO_EXPORT RGX_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef RGX_NO_DEPRECATED
#    define RGX_NO_DEPRECATED
#  endif
#endif

#endif /* RGX_EXPORT_H */
