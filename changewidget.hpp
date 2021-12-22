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

#ifndef CHANGEWIDGET_HPP
#define CHANGEWIDGET_HPP

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QWidget>

#include "changeentry.hpp"
#include "sqlmodel.hpp"

namespace Ui
{
	class ChangeWidget;
}

class ChangeWidget : public QWidget
{

		Q_OBJECT

	private:

		Ui::ChangeWidget* ui;
		QSqlDatabase& Database;

		QMap<int, QList<QVariantHash>> Unsaved;

		int Currentindex = 0;
		bool Locked = true;

	public:

		explicit ChangeWidget(QSqlDatabase& Db, QWidget* Parent = nullptr);
		virtual ~ChangeWidget(void) override;

		QList<QVariantHash> getChanges(int Index, const QSet<int> Filter = QSet<int>()) const;
		QList<QVariantHash> getChanges(const QSet<int> Filter = QSet<int>()) const;

		QIcon getIcon(int Status) const;

		void discardChanged(int Index);
		void saveChanges(int Index);

		bool isLocked(void) const;

	private slots:

		void updateStatus(void);

	public slots:

		void setDocIndex(int Index, bool Lock, bool Clear = true);

		void setLocked(bool Lock);

		void lock(void);
		void unlock(void);

		void appendChange(void);
		void removeChange(void);

		void undoChange(void);

	signals:

		void onChangesUpdate(int, int, int, int, int);

};

#endif // CHANGEWIDGET_HPP
