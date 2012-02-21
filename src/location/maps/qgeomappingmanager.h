/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGEOMAPPINGMANAGER_H
#define QGEOMAPPINGMANAGER_H

#include <QObject>
#include <QSize>
#include <QPair>
#include <QtLocation/qlocationglobal.h>
#include "qgeomaptype.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QLocale;
class QGeoBoundingBox;
class QGeoCameraCapabilities;
class QGeoCoordinate;
class QGeoMappingManagerPrivate;
class QGeoMapRequestOptions;
class QGeoMappingManagerEngine;
class QGeoTiledMapReply;

class QGeoTileSpec;
class QGeoMap;

class Q_LOCATION_EXPORT QGeoMappingManager : public QObject
{
    Q_OBJECT

public:
    enum CacheArea {
        DiskCache = 0x01,
        MemoryCache = 0x02,
        TextureCache = 0x04,
        AllCaches = 0xFF
    };
    Q_DECLARE_FLAGS(CacheAreas, CacheArea)

    ~QGeoMappingManager();

    QString managerName() const;
    int managerVersion() const;

    void registerMap(QGeoMap *map);
    void deregisterMap(QGeoMap *map);

    void updateTileRequests(QGeoMap *map,
                            const QSet<QGeoTileSpec> &tilesAdded,
                            const QSet<QGeoTileSpec> &tilesRemoved);

    QList<QGeoMapType> supportedMapTypes() const;

    int tileSize() const;

    bool isInitialized() const;

    QGeoCameraCapabilities cameraCapabilities() const;

    CacheAreas cacheHint() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

private Q_SLOTS:
    void engineTileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format);
    void engineTileError(const QGeoTileSpec &spec, const QString &errorString);

Q_SIGNALS:
    void tileError(const QGeoTileSpec &spec, const QString &errorString);
    void initialized();

private:
    QGeoMappingManager(QGeoMappingManagerEngine *engine, QObject *parent = 0);

    QGeoMappingManagerPrivate* d_ptr;
    Q_DISABLE_COPY(QGeoMappingManager)

    friend class QGeoServiceProvider;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoMappingManager::CacheAreas)

QT_END_NAMESPACE

QT_END_HEADER

#endif
