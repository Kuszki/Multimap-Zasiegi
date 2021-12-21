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

#include "indexdialog.hpp"
#include "ui_indexdialog.h"

IndexDialog::IndexDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::IndexDialog)
{
	ui->setupUi(this);

	updateStatus();
}

IndexDialog::~IndexDialog(void)
{
	delete ui;
}

void IndexDialog::accept(void)
{
	QDialog::accept();

	emit onRefresh(ui->pathEdit->text(),
				ui->pathCombo->currentIndex(),
				ui->recursiveCheck->isChecked());
}

void IndexDialog::openClicked(void)
{
	const QString Path = QFileDialog::getExistingDirectory(this, tr("Scan directory"));

	if (!Path.isEmpty()) ui->pathEdit->setText(Path);
}

void IndexDialog::updateStatus(void)
{
	const bool Path = !ui->pathEdit->text().isEmpty() && QDir(ui->pathEdit->text()).exists();
	const bool Rec = ui->pathCombo->currentIndex() == 2;

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(Path);
	ui->recursiveCheck->setEnabled(Rec);
}
