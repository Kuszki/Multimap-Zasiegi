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

#ifndef JOBWIDGET_HPP
#define JOBWIDGET_HPP

#include <QStandardItemModel>
#include <QSqlTableModel>
#include <QWidget>

#include <QtDebug>

#include "sqlmodel.hpp"

namespace Ui
{
	class JobWidget;
}

class JobWidget : public QWidget
{

		Q_OBJECT

	private:

		Ui::JobWidget* ui;

		SqlModel* model;

	public:

		explicit JobWidget(QSqlDatabase& Db, QWidget* Parent = nullptr);
		virtual ~JobWidget(void) override;

	private slots:

		void refreshClicked(void);

		void selectionChanged(const QModelIndex& Current);

	signals:

		void onIndexChange(int);

};

#endif // JOBWIDGET_HPP
