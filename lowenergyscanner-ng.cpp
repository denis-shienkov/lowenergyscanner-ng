#include "devicesmodel.h"
#include "servicesmodel.h"
#include "characteristicsmodel.h"
#include "descriptorsmodel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(BLE_DEVICES_MODEL, "scanner.devicesmodel")
Q_LOGGING_CATEGORY(BLE_SERVICES_MODEL, "scanner.servicesmodel")
Q_LOGGING_CATEGORY(BLE_CHARACTERISTICS_MODEL, "scanner.characteristicsmodel")
Q_LOGGING_CATEGORY(BLE_DESCRIPTORS_MODEL, "scanner.descriptorsmodel")

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));

    qmlRegisterType<DevicesModel>("qt.example.com", 1, 0, "DevicesModel");
    qmlRegisterType<ServicesModel>("qt.example.com", 1, 0, "ServicesModel");
    qmlRegisterType<CharacteriticsModel>("qt.example.com", 1, 0, "CharacteriticsModel");
    qmlRegisterType<DescriptorsModel>("qt.example.com", 1, 0, "DescriptorsModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/lowenergyscanner-ng.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
