
#pragma once

#include <QtGlobal>

#if defined(QTS3_LIBRARY)
#  define QTS3SHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTS3SHARED_EXPORT Q_DECL_IMPORT
#endif
