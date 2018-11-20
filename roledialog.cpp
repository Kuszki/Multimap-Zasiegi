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

#include "roledialog.hpp"
#include "ui_roledialog.h"

RoleDialog::RoleDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::RoleDialog)
{
	ui->setupUi(this);

	pathChanged(ui->pathEdit->text());
}

RoleDialog::~RoleDialog(void)
{
	delete ui;
}

void RoleDialog::accept(void)
{
	QDialog::accept();

	emit onRefresh(ui->typeCombo->currentIndex(),
				ui->pathEdit->text(),
				ui->appendCheck->isChecked());
}

void RoleDialog::openClicked(void)
{
	const QString Path = QFileDialog::getOpenFileName(this, tr("Load data"));

	if (!Path.isEmpty()) ui->pathEdit->setText(Path);
}

void RoleDialog::pathChanged(const QString& Path)
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!Path.isEmpty());
}
