// Copyright (C) Guangzhou FriendlyELEC Computer Tech. Co., Ltd.
// (http://www.friendlyarm.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, you can access it online at
// http://www.gnu.org/licenses/gpl-2.0.html.

#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QApplication> 
#include <QLabel> 
#include <QWSServer>
#endif
#include "mainwidget.h"
int main(int argc, char **argv) { 
	QApplication app (argc, argv); 
#if QT_VERSION >= 0x050000
	TMainWidget w(0,false,"https://github.com/friendlyarm/QtE-Demo.git");
#else
	QWSServer::setCursorVisible(true);
	TMainWidget w(0,false,"https://github.com/friendlyarm/QtE-Demo.git");
#endif
	w.setWindowFlags(Qt::FramelessWindowHint);
	w.showFullScreen();
	return app.exec(); 
}
