#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                         *
#*  Klient bazy danych projektu Multimap                                   *
#*  Copyright (C) 2018  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
#*                                                                         *
#*  This program is free software: you can redistribute it and/or modify   *
#*  it under the terms of the GNU General Public License as published by   *
#*  the  Free Software Foundation, either  version 3 of the  License, or   *
#*  (at your option) any later version.                                    *
#*                                                                         *
#*  This  program  is  distributed  in the hope  that it will be useful,   *
#*  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
#*  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
#*  GNU General Public License for more details.                           *
#*                                                                         *
#*  You should have  received a copy  of the  GNU General Public License   *
#*  along with this program. If not, see http://www.gnu.org/licenses/.     *
#*                                                                         *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

QT			+=	core gui widgets concurrent sql network qml

TARGET		=	Multimap-Zasiegi
TEMPLATE		=	app

CONFIG		+=	c++14

SOURCES		+=	main.cpp \
				mainwindow.cpp \
				aboutdialog.cpp \
				jobwidget.cpp \
				docwidget.cpp \
				indexdialog.cpp \
				sqlmodel.cpp \
				roledialog.cpp \
				changewidget.cpp \
				changeentry.cpp \
				settingsdialog.cpp \
				lockwidget.cpp

HEADERS		+=	mainwindow.hpp \
				aboutdialog.hpp \
				jobwidget.hpp \
				docwidget.hpp \
				indexdialog.hpp \
				sqlmodel.hpp \
				roledialog.hpp \
				changewidget.hpp \
				changeentry.hpp \
				settingsdialog.hpp \
				lockwidget.hpp

FORMS		+=	mainwindow.ui \
				aboutdialog.ui \
				jobwidget.ui \
				docwidget.ui \
				indexdialog.ui \
				roledialog.ui \
				changewidget.ui \
				changeentry.ui \
				settingsdialog.ui \
				lockwidget.ui

RESOURCES		+=	resources.qrc

TRANSLATIONS	+=	zasiegi_pl.ts
