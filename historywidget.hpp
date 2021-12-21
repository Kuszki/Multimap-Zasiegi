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

#ifndef HISTORYWIDGET_HPP
#define HISTORYWIDGET_HPP

#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QWidget>

#include "appcommon.hpp"

namespace Ui
{
	class HistoryWidget;
}

class HistoryWidget : public QWidget
{

		Q_OBJECT

	private:

		Ui::HistoryWidget* ui;
		QSqlDatabase& Database;

	public:

		explicit HistoryWidget(QSqlDatabase& Db, QWidget* Parent = nullptr);
		virtual ~HistoryWidget(void) override;

	private:

		void loadByDate(void);
		void loadByJob(void);

	private slots:

		void refreshClicked(void);

};

#endif // HISTORYWIDGET_HPP
