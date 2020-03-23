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

#include "integrationpluginpushnotifications.h"
#include "plugininfo.h"

#include "network/networkaccessmanager.h"
#include "nymeasettings.h"

#include <QJsonDocument>

IntegrationPluginPushNotifications::IntegrationPluginPushNotifications(QObject* parent): IntegrationPlugin (parent)
{
    // nymea:app community token. Can only be used to send notifications to nymea:app on
    // android/ios devices in combination with device specific tokens.
    m_firebaseServerToken = "AAAAcMswkws:APA91bGbM1sWlgd0Y-Rrodapmybn7WTMoMLw8FJIEHN3bwhon0_tLLKOiJ3YSDGm07lXTB1wRJBZRHeBLOWzkwrQQr85jLnvtdHl2M_QH-1R9OpMQpQa3H4hq1gl_FveKQ0bH5YTT4fz";

    // Allow overriding the firebase server token for products that require to keep it secret e.g. because bulk
    // notifications groups are enabled and knowing the server key would in theory allow spamming such groups
    QSettings settings(NymeaSettings::settingsPath() + "/nymead.conf", QSettings::IniFormat);
    settings.beginGroup("PushNotifications");
    if (settings.contains("firebaseServerToken")) {
        m_firebaseServerToken = settings.value("firebaseServerToken").toByteArray();
        QByteArray printedCopy = m_firebaseServerToken;
        qCDebug(dcPushNotifications()) << "Using custom Firebase server token:" << printedCopy.replace(printedCopy.length() - 30, 30, "******************************");
    }
    settings.endGroup();
}

IntegrationPluginPushNotifications::~IntegrationPluginPushNotifications()
{

}

void IntegrationPluginPushNotifications::setupThing(ThingSetupInfo *info)
{
    Thing *thing = info->thing();
    qCDebug(dcPushNotifications()) << "Setting up push notifications" << thing->name() << thing->id().toString();

    QString token = thing->paramValue(pushNotificationsThingTokenParamTypeId).toString();
    QString pushService = thing->paramValue(pushNotificationsThingServiceParamTypeId).toString();

    if (token.isEmpty()) {
        //: Error setting up thing
        return info->finish(Thing::ThingErrorMissingParameter, QT_TR_NOOP("The token must not be empty."));
    }
    QStringList availablePushServices = {"GCM", "APNs", "UBPorts"};
    if (!availablePushServices.contains(pushService)) {
        //: Error setting up thing
        return info->finish(Thing::ThingErrorMissingParameter, QT_TR_NOOP("The push service must not be empty."));
    }

    info->finish(Thing::ThingErrorNoError);
}

void IntegrationPluginPushNotifications::executeAction(ThingActionInfo *info)
{
    Thing *thing = info->thing();
    Action action = info->action();

    qCDebug(dcPushNotifications()) << "Executing action" << action.actionTypeId() << "for" << thing->name() << thing->id().toString();

    QString token = thing->paramValue(pushNotificationsThingTokenParamTypeId).toString();
    QString pushService = thing->paramValue(pushNotificationsThingServiceParamTypeId).toString();

    QString title = action.param(pushNotificationsNotifyActionTitleParamTypeId).value().toString();
    QString body = action.param(pushNotificationsNotifyActionBodyParamTypeId).value().toString();

    if (token.isEmpty()) {
        return info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("Push notifications need to be reconfigured."));
    }

    QNetworkRequest request;
    QVariantMap payload;
    if (pushService == "GCM") {
        request = QNetworkRequest(QUrl("https://fcm.googleapis.com/fcm/send"));
        request.setRawHeader("Authorization", "key=" + m_firebaseServerToken);
        request.setRawHeader("Content-Type", "application/json");

        QVariantMap notification;
        notification.insert("title", title);
        notification.insert("body", body);

        QVariantMap soundMap;
        soundMap.insert("sound", "default");

        QVariantMap android;
        android.insert("priority", "high");
        android.insert("notification", soundMap);

        payload.insert("to", token.toUtf8().trimmed());
        payload.insert("data", notification);
        payload.insert("android", android);

    } else if (pushService == "APNs") {
        request = QNetworkRequest(QUrl("https://fcm.googleapis.com/fcm/send"));
        request.setRawHeader("Authorization", "key=" + m_firebaseServerToken);
        request.setRawHeader("Content-Type", "application/json");

        QVariantMap notification;
        notification.insert("title", title);
        notification.insert("body", body);
        notification.insert("sound", "default");

        QVariantMap headers;
        headers.insert("apns-priority", "10");

        QVariantMap apns;
        apns.insert("headers", headers);

        payload.insert("to", token.toUtf8().trimmed());
        payload.insert("notification", notification);
        payload.insert("apns", apns);
    } else if (pushService == "UBPorts") {
        request = QNetworkRequest(QUrl("https://push.ubports.com/notify"));
        request.setRawHeader("Content-Type", "application/json");

        QVariantMap card;
        card.insert("icon", "notification");
        card.insert("summary", title);
        card.insert("body", body);
        card.insert("popup", true);
        card.insert("persist", true);

        QVariantMap notification;
        notification.insert("card", card);
        notification.insert("vibrate", true);
        notification.insert("sound", true);

        QVariantMap data;
        data.insert("notification", notification);

        payload.insert("data", data);
        payload.insert("appid", "io.guh.nymeaapp_nymea-app");
        payload.insert("expire_on", QDateTime::currentDateTime().toUTC().addMSecs(1000 * 60 * 10).toString(Qt::ISODate));
        payload.insert("token", token.toUtf8().trimmed());
    }

    qCDebug(dcPushNotifications()) << "Sending notification" << request.url().toString() << qUtf8Printable(QJsonDocument::fromVariant(payload).toJson());
    QNetworkReply *reply = hardwareManager()->networkManager()->post(request, QJsonDocument::fromVariant(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, info, [reply, pushService, info]{
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPushNotifications()) << "Push message sending failed for" << info->thing()->name() << info->thing()->id() << reply->errorString() << reply->error();
            emit info->finish(Thing::ThingErrorHardwareNotAvailable);
            return;
        }

        QByteArray data = reply->readAll();

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcPushNotifications()) << "Error reading reply from server for" << info->thing()->name() << info->thing()->id().toString() << error.errorString();
            qCWarning(dcPushNotifications()) << qUtf8Printable(data);
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        QVariantMap replyMap = jsonDoc.toVariant().toMap();
        qDebug(dcPushNotifications) << qUtf8Printable(jsonDoc.toJson());

        if (pushService == "GCM" || pushService == "APNs") {
            if (replyMap.value("success").toInt() != 1) {
                qCWarning(dcPushNotifications()) << "Error sending push notifcation:" << qUtf8Printable(jsonDoc.toJson());
                info->finish(Thing::ThingErrorHardwareFailure);
                return;
            }
        } else if (pushService == "UBPorts") {
            if (!replyMap.value("ok").toBool()) {
                qCWarning(dcPushNotifications()) << "Error sending push notifcation:" << qUtf8Printable(jsonDoc.toJson());
                info->finish(Thing::ThingErrorHardwareFailure);
                return;
            }
        }

        qCDebug(dcPushNotifications()) << "Message sent successfully";
        info->finish(Thing::ThingErrorNoError);
    });
}

