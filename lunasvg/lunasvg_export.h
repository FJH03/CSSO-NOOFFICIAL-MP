
#ifndef LUNASVG_EXPORT_H
#define LUNASVG_EXPORT_H

#ifdef LUNASVG_STATIC_DEFINE
#  define LUNASVG_EXPORT
#  define LUNASVG_NO_EXPORT
#else
#  ifndef LUNASVG_EXPORT
#    ifdef lunasvg_EXPORTS
        /* We are building this library */
#      define LUNASVG_EXPORT 
#    else
        /* We are using this library */
#      define LUNASVG_EXPORT 
#    endif
#  endif

#  ifndef LUNASVG_NO_EXPORT
#    define LUNASVG_NO_EXPORT 
#  endif
#endif

#ifndef LUNASVG_DEPRECATED
#  define LUNASVG_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef LUNASVG_DEPRECATED_EXPORT
#  define LUNASVG_DEPRECATED_EXPORT LUNASVG_EXPORT LUNASVG_DEPRECATED
#endif

#ifndef LUNASVG_DEPRECATED_NO_EXPORT
#  define LUNASVG_DEPRECATED_NO_EXPORT LUNASVG_NO_EXPORT LUNASVG_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LUNASVG_NO_DEPRECATED
#    define LUNASVG_NO_DEPRECATED
#  endif
#endif

#endif /* LUNASVG_EXPORT_H */
