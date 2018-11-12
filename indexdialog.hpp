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

#ifndef INDEXDIALOG_HPP
#define INDEXDIALOG_HPP

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDialog>

namespace Ui
{
	class IndexDialog;
}

class IndexDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::IndexDialog* ui;

	public:

		explicit IndexDialog(QWidget* Parent = nullptr);
		virtual ~IndexDialog(void) override;

	public slots:

		virtual void accept(void) override;

	private slots:

		void openClicked(void);
		void updateStatus(void);

	signals:

		void onRefresh(const QString&, int, bool);

};

#endif // INDEXDIALOG_HPP
