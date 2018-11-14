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

#include "lockwidget.hpp"
#include "ui_lockwidget.h"

LockWidget::LockWidget(QWidget* Parent)
: QWidget(Parent), ui(new Ui::LockWidget)
{
	ui->setupUi(this);

	model = new QStandardItemModel(0, 4, this);
	model->setHorizontalHeaderLabels(
	{
		tr("Sheet"), tr("Added"), tr("Removed"), tr("Modyfied")
	});

	ui->treeView->setModel(model);

	connect(ui->treeView, &QTreeView::doubleClicked,
		   this, &LockWidget::itemSelected);
}

LockWidget::~LockWidget(void)
{
	delete ui;
}

void LockWidget::itemSelected(const QModelIndex& Index)
{
	const auto Data = model->index(Index.row(), 0, Index.parent());

	if (Index.parent().isValid())
	{
		emit onDocSelected(model->data(Data, Qt::UserRole).toInt());
	}
	else emit onJobSelected(model->data(Data, Qt::UserRole).toInt());
}
