/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef INTEGRATIONPLUGINKEBA_H
#define INTEGRATIONPLUGINKEBA_H

#include "integrations/integrationplugin.h"
#include "plugintimer.h"
#include "kecontact.h"
#include "discovery.h"
#include "host.h"

#include <QHash>
#include <QNetworkReply>
#include <QUdpSocket>
#include <QDateTime>

class IntegrationPluginKeba : public IntegrationPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "io.nymea.IntegrationPlugin" FILE "integrationpluginkeba.json")
    Q_INTERFACES(IntegrationPlugin)

public:
    explicit IntegrationPluginKeba();

    void discoverThings(ThingDiscoveryInfo *info) override;
    void setupThing(ThingSetupInfo *info) override;

    void postSetupThing(Thing* thing) override;
    void thingRemoved(Thing* thing) override;

    void executeAction(ThingActionInfo *info) override;
    void updateData();

private:
    PluginTimer *m_pluginTimer = nullptr;
    QHash<ThingId, KeContact *> m_kebaDevices;
    QHash<KeContact *, ThingSetupInfo *> m_asyncSetup;
    QHash<QUuid, ThingActionInfo *> m_asyncActions;
    QHash<ThingId, QDateTime> m_chargingSessionStartTime;

    void setDeviceState(Thing *device, KeContact::State state);
    void setDevicePlugState(Thing *device, KeContact::PlugState plugState);

private slots:
    void onConnectionChanged(bool status);
    void onCommandExecuted(QUuid requestId, bool success);
    void onReportOneReceived(const KeContact::ReportOne &reportOne);
    void onReportTwoReceived(const KeContact::ReportTwo &reportTwo);
    void onReportThreeReceived(const KeContact::ReportThree &reportThree);
    void onBroadcastReceived(KeContact::BroadcastType type, const QVariant &content);
};

#endif // INTEGRATIONPLUGINKEBA_H
