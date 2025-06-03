#ifndef PRIVORA_QT_AUTOMINTNOTIFICATION_H
#define PRIVORA_QT_AUTOMINTNOTIFICATION_H

#include "sparkmodel.h"
#include "walletmodel.h"

#include <QDialog>

namespace Ui {
    class AutomintNotification;
}


class AutomintSparkNotification : public QDialog
{
    Q_OBJECT;

public:
    explicit AutomintSparkNotification(QWidget *parent = 0);
    ~AutomintSparkNotification();

public:
    void setModel(WalletModel *model);

Q_SIGNALS:
    void ackMintSparkAll(AutoMintSparkAck, CAmount, QString);

public Q_SLOTS:
    bool close();

private Q_SLOTS:
    void accept();
    void reject();

private:
    Ui::AutomintNotification *ui;
    SparkModel *sparkModel;
};

#endif // PRIVORA_QT_AUTOMINTNOTIFICATION_H