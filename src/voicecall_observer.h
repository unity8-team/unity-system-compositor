/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#ifndef USC_VOICECALL_OBSERVER_H_
#define USC_VOICECALL_OBSERVER_H_

#include <QObject>

class QDBusObjectPath;

namespace usc
{

class CallStateListener;

class VoiceCallObserver : public QObject
{
    Q_OBJECT
public:
    VoiceCallObserver(CallStateListener& listener);

private Q_SLOTS:
    void call_added();
    void call_removed();
    void modem_added(QDBusObjectPath const&, QVariantMap const&);

private:
    void query_existing_modems();
    void subscribe_to_call_manager(QString const& modem);
    void subscribe_to_modem_manager();

    CallStateListener& listener;
    int active_calls;
};

}

#endif

