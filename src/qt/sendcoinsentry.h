// Copyright (c) 2011-2015 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_QT_SENDCOINSENTRY_H
#define PRIVORA_QT_SENDCOINSENTRY_H

#include "walletmodel.h"

#include <QStackedWidget>

class WalletModel;
class PlatformStyle;

namespace Ui {
    class SendCoinsEntry;
}

/**
 * A single entry in the dialog for sending privoras.
 * Stacked widget, with different UIs for payment requests
 * with a strong payee identity.
 */
class SendCoinsEntry : public QStackedWidget
{
    Q_OBJECT

public:
    explicit SendCoinsEntry(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SendCoinsEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendCoinsRecipient getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();
    /** Needs validate() to be called before calling isPayToPcode()*/
    bool isPayToPcode() const;

    void setValue(const SendCoinsRecipient &value);
    void setAddress(const QString &address);
    void setSubtractFeeFromAmount(bool enable);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases
     *  (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();
    void setWarning(bool fAnonymousMode);
    void setfAnonymousMode(bool fAnonymousMode);
    static QString generateWarningText(const QString& address, const bool fAnonymousMode);

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void removeEntry(SendCoinsEntry *entry);
    void payAmountChanged();
    void subtractFeeFromAmountChanged();

private Q_SLOTS:
    void deleteClicked();
    void on_payTo_textChanged(const QString &address);
    void on_MemoTextChanged(const QString &text);
    void on_addressBookButton_clicked();
    void on_pasteButton_clicked();
    void updateDisplayUnit();

private:
    SendCoinsRecipient recipient;
    Ui::SendCoinsEntry *ui;
    WalletModel *model;
    const PlatformStyle *platformStyle;
    bool isPcodeEntry;
    bool fAnonymousMode = false;
    bool updateLabel(const QString &address);
    void resizeEvent(QResizeEvent* event) override;
    void adjustTextSize(int width, int height);
    
};

#endif // PRIVORA_QT_SENDCOINSENTRY_H
