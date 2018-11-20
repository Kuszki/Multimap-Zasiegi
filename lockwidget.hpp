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

#ifndef LOCKWIDGET_HPP
#define LOCKWIDGET_HPP

#include <QStandardItemModel>
#include <QWidget>

namespace Ui
{
	class LockWidget;
}

class LockWidget : public QWidget
{

		Q_OBJECT

	private:

		Ui::LockWidget* ui;
		QStandardItemModel* model;

	public:

		explicit LockWidget(QWidget* Parent = nullptr);
		virtual ~LockWidget(void) override;

	public slots:

		void appendDocument(const QString& File, int dID,
						const QString& Job, int jID);

		void removeDocument(int Index);

		void recalcChanges(int Index, int Add, int Del, int Mod, int Err);

	private slots:

		void itemSelected(const QModelIndex& Index);

	signals:

		void onDocSelected(int, int);

};

#endif // LOCKWIDGET_HPP
