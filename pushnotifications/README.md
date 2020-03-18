# Push Notifications

This plugin allows to send push notifications to mobile devices.

## Supported platforms

* Android
* iOS
* Ubuntu/UBPorts Phone

## Requirements

A push notification device token is required during setup. For Android *and* iOS, a Firebase
token is required. Note that native iOS push tokens are not supported as the Apple Push Notification
Service (APNs) does not allow sending messages in such a distributed manner, however, Firebase is available
for iOS too. On Ubuntu, the UBPorts push services are used.

## More

During setup, the token and the push service system needs to be provided. As it is impossible for an end
user to obtain this token, a client app typically would implicitly add/remove such push notification things
depending on the user's preferences on notification.

