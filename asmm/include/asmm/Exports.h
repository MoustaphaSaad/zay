
#ifndef ASMM_EXPORT_H
#define ASMM_EXPORT_H

#ifdef ASMM_STATIC_DEFINE
#  define ASMM_EXPORT
#  define ASMM_NO_EXPORT
#else
#  ifndef ASMM_EXPORT
#    ifdef asmm_EXPORTS
        /* We are building this library */
#      define ASMM_EXPORT 
#    else
        /* We are using this library */
#      define ASMM_EXPORT 
#    endif
#  endif

#  ifndef ASMM_NO_EXPORT
#    define ASMM_NO_EXPORT 
#  endif
#endif

#ifndef ASMM_DEPRECATED
#  define ASMM_DEPRECATED __declspec(deprecated)
#endif

#ifndef ASMM_DEPRECATED_EXPORT
#  define ASMM_DEPRECATED_EXPORT ASMM_EXPORT ASMM_DEPRECATED
#endif

#ifndef ASMM_DEPRECATED_NO_EXPORT
#  define ASMM_DEPRECATED_NO_EXPORT ASMM_NO_EXPORT ASMM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ASMM_NO_DEPRECATED
#    define ASMM_NO_DEPRECATED
#  endif
#endif

#endif /* ASMM_EXPORT_H */
