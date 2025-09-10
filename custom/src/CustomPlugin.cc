#include "CustomPlugin.h"
#include <QDebug>

CustomPlugin::CustomPlugin(QGCApplication* app, QGCToolbox* toolbox)
    : QGCCorePlugin(app, toolbox)
{
    qDebug() << "[CustomPlugin] constructed";
    // No GeoWork registration here — handled in GeoWork_qmlinit.cpp
}
