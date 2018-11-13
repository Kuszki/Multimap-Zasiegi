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

#include "sqlmodel.hpp"

SqlModel::SqlModel(QObject* Parent, QSqlDatabase Db)
: QSqlRelationalTableModel(Parent, Db) {}

SqlModel::~SqlModel(void) {}

QSet<int> SqlModel::getEditable(void) const
{
	return Editable;
}

QVariant SqlModel::getUid(const QModelIndex& Index) const
{
	return data(index(Index.row(), 0, Index.parent()));
}

QModelIndex SqlModel::findByUid(const QVariant& Uid) const
{
	for (int i = 0, N = rowCount(); i < N; ++i)
	{
		QModelIndex Current = index(i, 0);

		if (data(Current) == Uid) return Current;
	}

	return QModelIndex();
}

Qt::ItemFlags SqlModel::flags(const QModelIndex& Index) const
{
	auto Flag = QSqlRelationalTableModel::flags(Index);

	if (Editable.contains(Index.column())) return Flag;
	else return Flag & ~Qt::ItemIsEditable;
}

void SqlModel::setEditable(const QSet<int>& Val)
{
	Editable = Val;
}
