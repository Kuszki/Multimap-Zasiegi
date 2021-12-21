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

#ifndef CHANGEENTRY_HPP
#define CHANGEENTRY_HPP

#include <QStringListModel>
#include <QSqlDatabase>
#include <QVariantHash>
#include <QSqlQuery>
#include <QWidget>

namespace Ui
{
	class ChangeEntry;
}

class ChangeEntry : public QWidget
{

		Q_OBJECT

	private:

		Ui::ChangeEntry* ui;

		int UID, DID;
		bool Locked;
		int Status;

		QVariantHash Origin;

	public:

		explicit ChangeEntry(const QVariantHash& Data, QSqlDatabase& Db, bool Lock = false, QWidget* Parent = nullptr);
		virtual ~ChangeEntry(void) override;

		QVariantHash getOrigin(void) const;
		QVariantHash getData(void) const;

		void setOrigin(const QVariantHash& Data);
		void setData(const QVariantHash& Data);

		bool isChanged(void) const;
		bool isValid(void) const;
		bool isLocked(void) const;

	public slots:

		void setLocked(bool Lock);

		void lock(void);
		void unlock(void);

	private slots:

		void addRightClicked(void);
		void addLeftClicked(void);

		void delRightClicked(void);
		void delLeftClicked(void);

		void updateStatus(void);

	signals:

		void onStatusUpdate(int);

};

#endif // CHANGEENTRY_HPP
