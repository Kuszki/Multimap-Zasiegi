/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Klient bazy danych projektu Multimap                                   *
 *  Copyright (C) 2018  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtConcurrent>
#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "settingsdialog.hpp"
#include "aboutdialog.hpp"
#include "indexdialog.hpp"
#include "roledialog.hpp"

#include "changewidget.hpp"
#include "lockwidget.hpp"
#include "jobwidget.hpp"
#include "docwidget.hpp"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{

		Q_OBJECT

	private:

		Ui::MainWindow* ui;
		QSqlDatabase Db;

		QVariantMap Options;

		ChangeWidget* cwidget;
		LockWidget* lwidget;
		JobWidget* jwidget;
		DocWidget* dwidget;

		QDockWidget* changes;
		QDockWidget* locks;
		QDockWidget* jobs;
		QDockWidget* docs;

		QPixmap Image;

		QList<QPair<int, int>> Queue;
		QSet<int> Locked;

		int CurrentDoc = 0;

		double Scale = 1.0;
		int Rotation = 0;

	public:

		explicit MainWindow(QWidget* Parent = nullptr);
		virtual ~MainWindow(void) override;

	private:

		QString getCurrentUser(void) const;

	private slots:

		void aboutClicked(void);
		void scanClicked(void);
		void rolesClicked(void);
		void settingsClicked(void);

		void nextClicked(void);
		void prevClicked(void);
		void saveClicked(void);
		void editClicked(void);
		void lockClicked(void);
		void unlockClicked(void);

		void zoomInClicked(void);
		void zoomOutClicked(void);
		void zoomOrgClicked(void);
		void zoomFitClicked(void);

		void rotateLeftClicked(void);
		void rotateRightClicked(void);

		void changeAddClicked(void);
		void changeDelClicked(void);
		void changeUndoClicked(void);

		void documentChanged(int Index);

		void focusDocument(int Document, int Job);

		void updateImage(const QString& Path);

		void saveSettings(const QVariantMap& Settings);

		void scanDirectory(const QString& Dir,
					    int Mode, bool Rec);

		void updateRoles(const QString& Path,
					  bool Addnew);

	signals:

		void onSaveChanges(int);

};

#endif // MAINWINDOW_HPP
