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

#ifndef SQLMODEL_HPP
#define SQLMODEL_HPP

#include <QSqlRelationalTableModel>
#include <QSet>

class SqlModel : public QSqlRelationalTableModel
{

	private:

		QSet<int> Editable;

	public:

		explicit SqlModel(QObject* Parent = nullptr, QSqlDatabase Db = QSqlDatabase());
		virtual ~SqlModel(void) override;

		void setEditable(const QSet<int>& Val);
		QSet<int> getEditable(void) const;

		QVariant getUid(const QModelIndex& Index) const;

		QModelIndex findByUid(const QVariant& Uid) const;

		virtual Qt::ItemFlags flags(const QModelIndex& Index) const override;
};

#endif // SQLMODEL_HPP
