/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QtCore/QMetaType>
#include <QString>
#include <QtTest/QtTest>

#include <qgeoserviceprovider.h>
#include <qplacemanager.h>


#ifndef WAIT_UNTIL
#define WAIT_UNTIL(__expr) \
        do { \
        const int __step = 50; \
        const int __timeout = 5000; \
        if (!(__expr)) { \
            QTest::qWait(0); \
        } \
        for (int __i = 0; __i < __timeout && !(__expr); __i+=__step) { \
            QTest::qWait(__step); \
        } \
    } while (0)
#endif

QT_USE_NAMESPACE

class tst_QPlaceManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void compatiblePlace();
    void testMetadata();
    void testLocales();
    void testMatchUnsupported();

private:
    bool checkSignals(QPlaceReply *reply, QPlaceReply::Error expectedError);

    QGeoServiceProvider *provider;
    QPlaceManager *placeManager;
};

void tst_QPlaceManager::initTestCase()
{
    provider = 0;

    QStringList providers = QGeoServiceProvider::availableServiceProviders();
    QVERIFY(providers.contains("qmlgeo.test.plugin"));

    provider = new QGeoServiceProvider("qmlgeo.test.plugin");
    provider->setAllowExperimental(true);
    QCOMPARE(provider->placesFeatures() & QGeoServiceProvider::OfflinePlacesFeature,
             QGeoServiceProvider::OfflinePlacesFeature);
    placeManager = provider->placeManager();
    QVERIFY(placeManager);
}

void tst_QPlaceManager::testMetadata()
{
    QCOMPARE(placeManager->managerName(), QLatin1String("qmlgeo.test.plugin"));
    QCOMPARE(placeManager->managerVersion(), 100);
}

void tst_QPlaceManager::testLocales()
{
    QCOMPARE(placeManager->locales().count(), 1);
    QCOMPARE(placeManager->locales().at(0), QLocale());

    QLocale locale(QLocale::Norwegian, QLocale::Norway);
    placeManager->setLocale(locale);

    QCOMPARE(placeManager->locales().at(0), locale);

    QList<QLocale> locales;
    QLocale en_AU = QLocale(QLocale::English, QLocale::Australia);
    QLocale en_UK = QLocale(QLocale::English, QLocale::UnitedKingdom);
    locales << en_AU << en_UK;
    placeManager->setLocales(locales);
    QCOMPARE(placeManager->locales().count(), 2);
    QCOMPARE(placeManager->locales().at(0), en_AU);
    QCOMPARE(placeManager->locales().at(1), en_UK);
}

void tst_QPlaceManager::testMatchUnsupported()
{
    QPlaceMatchRequest request;
    QPlaceMatchReply *reply = placeManager->matchingPlaces(request);
    QVERIFY(checkSignals(reply, QPlaceReply::UnsupportedError));
}

void tst_QPlaceManager::compatiblePlace()
{
    QPlace place;
    place.setPlaceId(QLatin1String("4-8-15-16-23-42"));
    place.setName(QLatin1String("Island"));
    place.setVisibility(QtLocation::PublicVisibility);

    QPlace compatPlace = placeManager->compatiblePlace(place);
    QVERIFY(compatPlace.placeId().isEmpty());
    QCOMPARE(compatPlace.name(), QLatin1String("Island"));
    QCOMPARE(compatPlace.visibility(), QtLocation::UnspecifiedVisibility);
}

void tst_QPlaceManager::cleanupTestCase()
{
    delete provider;
}

bool tst_QPlaceManager::checkSignals(QPlaceReply *reply, QPlaceReply::Error expectedError)
{
    QSignalSpy finishedSpy(reply, SIGNAL(finished()));
    QSignalSpy errorSpy(reply, SIGNAL(error(QPlaceReply::Error,QString)));
    QSignalSpy managerFinishedSpy(placeManager, SIGNAL(finished(QPlaceReply*)));
    QSignalSpy managerErrorSpy(placeManager,SIGNAL(error(QPlaceReply*,QPlaceReply::Error,QString)));

    if (expectedError != QPlaceReply::NoError) {
        //check that we get an error signal from the reply
        WAIT_UNTIL(errorSpy.count() == 1);
        if (errorSpy.count() != 1) {
            qWarning() << "Error signal for search operation not received";
            return false;
        }

        //check that we get the correct error from the reply's signal
        QPlaceReply::Error actualError = qvariant_cast<QPlaceReply::Error>(errorSpy.at(0).at(0));
        if (actualError != expectedError) {
            qWarning() << "Actual error code in reply signal does not match expected error code";
            qWarning() << "Actual error code = " << actualError;
            qWarning() << "Expected error coe =" << expectedError;
            return false;
        }

        //check that we get an error  signal from the manager
        WAIT_UNTIL(managerErrorSpy.count() == 1);
        if (managerErrorSpy.count() !=1) {
           qWarning() << "Error signal from manager for search operation not received";
           return false;
        }

        //check that we get the correct reply instance in the error signal from the manager
        if (qvariant_cast<QPlaceReply*>(managerErrorSpy.at(0).at(0)) != reply)  {
            qWarning() << "Reply instance in error signal from manager is incorrect";
            return false;
        }

        //check that we get the correct error from the signal of the manager
        actualError = qvariant_cast<QPlaceReply::Error>(managerErrorSpy.at(0).at(1));
        if (actualError != expectedError) {
            qWarning() << "Actual error code from manager signal does not match expected error code";
            qWarning() << "Actual error code =" << actualError;
            qWarning() << "Expected error code = " << expectedError;
            return false;
        }
    }

    //check that we get a finished signal
    WAIT_UNTIL(finishedSpy.count() == 1);
    if (finishedSpy.count() !=1) {
        qWarning() << "Finished signal from reply not received";
        return false;
    }

    if (reply->error() != expectedError) {
        qWarning() << "Actual error code does not match expected error code";
        qWarning() << "Actual error code: " << reply->error();
        qWarning() << "Expected error code" << expectedError;
        return false;
    }

    if (expectedError == QPlaceReply::NoError && !reply->errorString().isEmpty()) {
        qWarning() << "Expected error was no error but error string was not empty";
        qWarning() << "Error string=" << reply->errorString();
        return false;
    }

    //check that we get the finished signal from the manager
    WAIT_UNTIL(managerFinishedSpy.count() == 1);
    if (managerFinishedSpy.count() != 1) {
        qWarning() << "Finished signal from manager not received";
        return false;
    }

    //check that the reply instance in the finished signal from the manager is correct
    if (qvariant_cast<QPlaceReply *>(managerFinishedSpy.at(0).at(0)) != reply) {
        qWarning() << "Reply instance in finished signal from manager is incorrect";
        return false;
    }

    return true;
}

QTEST_GUILESS_MAIN(tst_QPlaceManager)

#include "tst_qplacemanager.moc"
