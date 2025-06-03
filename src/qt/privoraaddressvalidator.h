// Copyright (c) 2011-2014 The Privora Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVORA_QT_PRIVORAADDRESSVALIDATOR_H
#define PRIVORA_QT_PRIVORAADDRESSVALIDATOR_H

#include <QValidator>

#ifdef ENABLE_WALLET
#include "../spark/sparkwallet.h"
#endif

#include "../spark/state.h"

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class PrivoraAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PrivoraAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Privora address widget validator, checks for a valid privora address.
 */
class PrivoraAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PrivoraAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;

    bool validateSparkAddress(const std::string& address) const;
};

#endif // PRIVORA_QT_PRIVORAADDRESSVALIDATOR_H
