/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Klient bazy danych projektu Multimap                                   *
 *  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
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
#include "importdialog.hpp"
#include "aboutdialog.hpp"
#include "indexdialog.hpp"
#include "roledialog.hpp"

#include "historywidget.hpp"
#include "changewidget.hpp"
#include "lockwidget.hpp"
#include "jobwidget.hpp"
#include "docwidget.hpp"

#include "imagedock.hpp"

#include "appcommon.hpp"

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

		QVariantHash Options;

		HistoryWidget* hwidget;
		ChangeWidget* cwidget;
		LockWidget* lwidget;
		JobWidget* jwidget;
		DocWidget* dwidget;

		ImageDock* image;

		QDockWidget* history;
		QDockWidget* changes;
		QDockWidget* locks;
		QDockWidget* jobs;
		QDockWidget* docs;

		QList<int> Queue;
		QSet<int> Locked;

		int CurrentJob = 0;

		double Scale = 1.0;
		int Rotation = 0;

	public:

		explicit MainWindow(QWidget* Parent = nullptr);
		virtual ~MainWindow(void) override;

	private:

		void updateDocRoles(const QString& Path, bool Addnew);
		void updateJobRoles(const QString& Path, bool Addnew);

	private slots:

		void aboutClicked(void);
		void importdataClicked(void);
		void scanClicked(void);
		void rolesClicked(void);
		void importClicked(void);
		void exportClicked(void);
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

		void openfileClicked(void);
		void openfolderClicked(void);

		void changeAddClicked(void);
		void changeDelClicked(void);
		void changeUndoClicked(void);

		void dockOptionsChanged(void);

		void jobChanged(int Index);
		void documentChanged(int Index);

		void focusDocument(int Job);

		void updateImage(const QString& Path);

		void saveSettings(const QVariantHash& Settings);

		void scanDirectory(const QString& Dir,
					    int Mode, bool Rec);

		void updateRoles(int Objecttype,
					  const QString& Path,
					  bool Addnew);

		void importData(const QVariantHash& Jobs,
					 const QVariantHash& Docs);

	signals:

		void onSaveChanges(int);

};

#endif // MAINWINDOW_HPP
