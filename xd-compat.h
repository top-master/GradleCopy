
/*
 * XD is an entire separate framework based on Qt.
 *
 * This file tries to provide few of XD's helpers to Qt users.
 */

#include <QtCore/qglobal.h>

#if defined(QT_HAS_XD) && QT_HAS_XD(4, 8, 0)
// Nothing to do.
#elif !defined(XD_COMPAT)
#define XD_COMPAT

#ifndef Q_DECL_CONSTEXPR
#  define Q_DECL_CONSTEXPR constexpr
#  define Q_DECL_RELAXED_CONSTEXPR constexpr
#endif
#ifndef Q_CONSTEXPR
#  define Q_CONSTEXPR constexpr
#  define Q_RELAXED_CONSTEXPR constexpr
#endif

#ifndef Q_DECL_OVERRIDE
#  define Q_DECL_OVERRIDE override
#endif
#ifndef Q_DECL_FINAL
#  define Q_DECL_FINAL final
#endif

#ifndef Q_DECL_NOEXCEPT
#  define Q_DECL_NOEXCEPT noexcept
#  define Q_DECL_NOEXCEPT_EXPR(x) noexcept(x)
#endif

// Override with C++11 noexcept.
#ifdef Q_DECL_NOTHROW
#  undef Q_DECL_NOTHROW
#endif
#define Q_DECL_NOTHROW Q_DECL_NOEXCEPT


// Usable with Lambda, like `QT_FINALLY([&] { qFree(myVariable) });`.
#define QT_FINALLY_X(name, callback) auto name = qScopeGuard(callback); \
    do { Q_UNUSED(name) } while(0)
#define QT_FINALLY(callback) QT_FINALLY_X(QT_JOIN(_qDefer, __LINE__), callback)


#ifdef __cplusplus

namespace QtPrivate {
    template <typename Func1>
    class QFinally {
        Func1 data;
    public:
        Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QFinally(Func1 &&f)
            : data(qMove(f))
        {}

        Q_ALWAYS_INLINE ~QFinally() {
            (data)();
        }
    };
} // namespace QtPrivate

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

// Under LGPL-3 header permission: uses Qt-6's "qScopeGuard" name.
template <typename Func1>
Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QtPrivate::QFinally<Func1 > qScopeGuard(Func1 && f)
{
    return QtPrivate::QFinally<Func1 >(qMove(f));
}

#endif // QT_VERSION 5

#endif // __cplusplus end

#endif // XD_COMPAT
