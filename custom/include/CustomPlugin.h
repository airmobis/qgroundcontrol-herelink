#pragma once
#include "QGCCorePlugin.h"

class CustomPlugin : public QGCCorePlugin {
    Q_OBJECT
public:
    CustomPlugin(QGCApplication* app, QGCToolbox* toolbox);
};
