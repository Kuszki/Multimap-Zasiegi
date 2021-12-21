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

#ifndef DOCWIDGET_HPP
#define DOCWIDGET_HPP

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QStandardItemModel>
#include <QWidget>

#include "sqlmodel.hpp"

namespace Ui
{
	class DocWidget;
}

class DocWidget : public QWidget
{

		Q_OBJECT

	private:

		Ui::DocWidget* ui;

		QString Currpath;
		SqlModel* model;

		int Currjob = 0;
		int Currdoc = 0;

	public:

		explicit DocWidget(QSqlDatabase& Db, QWidget* Parent = nullptr);
		virtual ~DocWidget(void) override;

		QString currentImage(void) const;

		int jobIndex(void) const;
		int docIndex(void) const;

	public slots:

		void setVisibleHeaders(const QVariantList& List);

		void setJobIndex(int Index);
		void setDocIndex(int Index);

		void updateData(int Index);

	private slots:

		void selectionChanged(const QModelIndex& Current);

	signals:

		void onPathChange(const QString&);
		void onIndexChange(int);

};

#endif // DOCWIDGET_HPP
