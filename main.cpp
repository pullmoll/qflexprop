/*****************************************************************************
 *
 * Qt5 Propeller 2 main program
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "qflexprop.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(QLatin1String("QFlexProp"));
    a.setApplicationVersion(QString("%1.%2.%3")
			    .arg(VERSION_MAJOR)
			    .arg(VERSION_MINOR)
			    .arg(VERSION_PATCH));
    a.setOrganizationName(QLatin1String("pullmoll"));
    a.setOrganizationDomain(QLatin1String("pullmoll.github.io"));

    QFlexProp w;
    w.show();
    return a.exec();
}
