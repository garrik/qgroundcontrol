/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "LogDownloadTest.h"
#include "LogDownloadController.h"
#include "LogEntry.h"
#include "MockLink.h"
#include "MultiSignalSpy.h"

#include <QtCore/QDir>

LogDownloadTest::LogDownloadTest(void)
{

}

void LogDownloadTest::downloadTest(void)
{

    _connectMockLink(MAV_AUTOPILOT_PX4);

    LogDownloadController* controller = new LogDownloadController();

    _rgLogDownloadControllerSignals[requestingListChangedSignalIndex] =     SIGNAL(requestingListChanged());
    _rgLogDownloadControllerSignals[downloadingLogsChangedSignalIndex] =    SIGNAL(downloadingLogsChanged());
    _rgLogDownloadControllerSignals[modelChangedSignalIndex] =              SIGNAL(modelChanged());

    _multiSpyLogDownloadController = new MultiSignalSpy();
    QVERIFY(_multiSpyLogDownloadController->init(controller, _rgLogDownloadControllerSignals, _cLogDownloadControllerSignals));

    controller->refresh();
    QVERIFY(_multiSpyLogDownloadController->waitForSignalByIndex(requestingListChangedSignalIndex, 10000));
    _multiSpyLogDownloadController->clearAllSignals();
    if (controller->requestingList()) {
        QVERIFY(_multiSpyLogDownloadController->waitForSignalByIndex(requestingListChangedSignalIndex, 10000));
        QCOMPARE(controller->requestingList(), false);
    }
    _multiSpyLogDownloadController->clearAllSignals();

    auto model = controller->model();
    QVERIFY(model);
    qDebug() << model->count();
    model->value<QGCLogEntry*>(0)->setSelected(true);

    QString downloadTo = QDir::currentPath();
    qDebug() << "download to:" << downloadTo;
    controller->downloadToDirectory(downloadTo);
    QVERIFY(_multiSpyLogDownloadController->waitForSignalByIndex(downloadingLogsChangedSignalIndex, 10000));
    _multiSpyLogDownloadController->clearAllSignals();
    if (controller->downloadingLogs()) {
        QVERIFY(_multiSpyLogDownloadController->waitForSignalByIndex(downloadingLogsChangedSignalIndex, 10000));
        QCOMPARE(controller->downloadingLogs(), false);
    }
    _multiSpyLogDownloadController->clearAllSignals();

    QString downloadFile = QDir(downloadTo).filePath("log_0_UnknownDate.ulg");
    QVERIFY(UnitTest::fileCompare(downloadFile, _mockLink->logDownloadFile()));

    QFile::remove(downloadFile);

    delete controller;
}
