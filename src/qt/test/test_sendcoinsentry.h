#ifndef PRIVORA_QT_TEST_SENDCOINSENTRY_H
#define PRIVORA_QT_TEST_SENDCOINSENTRY_H

#include <QObject>
#include <QTest>
#include "sendcoinsentry.h"

class TestSendCoinsEntry : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGenerateWarningText();
};

#endif // PRIVORA_QT_TEST_SENDCOINSENTRY_H