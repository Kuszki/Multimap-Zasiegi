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

#ifndef IMPORTDIALOG_HPP
#define IMPORTDIALOG_HPP

#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QDialog>
#include <QFile>
#include <QSet>

namespace Ui
{
	class ImportDialog;
}

class ImportDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::ImportDialog* ui;

		QVariantMap Jobs;
		QVariantMap Docs;

		QStandardItemModel* jmodel;
		QStandardItemModel* dmodel;

	public:

		explicit ImportDialog(QWidget* Parent = nullptr);
		virtual ~ImportDialog(void) override;

	private:

		void updateData(const QString& Path);

	public slots:

		virtual void accept(void) override;

	private slots:

		void openClicked(void);
		void pathChanged(const QString& Path);

	signals:

		void onLoadRequest(const QVariantMap&, const QVariantMap&);

};

#endif // IMPORTDIALOG_HPP
