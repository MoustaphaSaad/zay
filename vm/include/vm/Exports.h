
#ifndef VM_EXPORT_H
#define VM_EXPORT_H

#ifdef VM_STATIC_DEFINE
#  define VM_EXPORT
#  define VM_NO_EXPORT
#else
#  ifndef VM_EXPORT
#    ifdef vm_EXPORTS
        /* We are building this library */
#      define VM_EXPORT 
#    else
        /* We are using this library */
#      define VM_EXPORT 
#    endif
#  endif

#  ifndef VM_NO_EXPORT
#    define VM_NO_EXPORT 
#  endif
#endif

#ifndef VM_DEPRECATED
#  define VM_DEPRECATED __declspec(deprecated)
#endif

#ifndef VM_DEPRECATED_EXPORT
#  define VM_DEPRECATED_EXPORT VM_EXPORT VM_DEPRECATED
#endif

#ifndef VM_DEPRECATED_NO_EXPORT
#  define VM_DEPRECATED_NO_EXPORT VM_NO_EXPORT VM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VM_NO_DEPRECATED
#    define VM_NO_DEPRECATED
#  endif
#endif

#endif /* VM_EXPORT_H */
