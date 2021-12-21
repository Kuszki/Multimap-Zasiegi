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

#include "lockwidget.hpp"
#include "ui_lockwidget.h"

LockWidget::LockWidget(QWidget* Parent)
: QWidget(Parent), ui(new Ui::LockWidget)
{
	ui->setupUi(this);

	model = new QStandardItemModel(0, 5, this);
	model->setHorizontalHeaderLabels({ tr("Job"), "", "", "", "" });

	model->horizontalHeaderItem(1)->setIcon(QIcon::fromTheme("list-add"));
	model->horizontalHeaderItem(2)->setIcon(QIcon::fromTheme("list-remove"));
	model->horizontalHeaderItem(3)->setIcon(QIcon::fromTheme("tools-check-spelling"));
	model->horizontalHeaderItem(4)->setIcon(QIcon::fromTheme("dialog-error"));

	ui->treeView->setModel(model);
	ui->treeView->setColumnWidth(1, 50);
	ui->treeView->setColumnWidth(2, 50);
	ui->treeView->setColumnWidth(3, 50);
	ui->treeView->setColumnWidth(4, 50);

	connect(ui->treeView, &QTreeView::doubleClicked,
		   this, &LockWidget::itemSelected);
}

LockWidget::~LockWidget(void)
{
	delete ui;
}

void LockWidget::appendDocument(const QString& Job, int jID)
{
	QList<QStandardItem*> Row;

	auto* Doc = new QStandardItem(Job);
	Doc->setData(jID, Qt::UserRole);
	Row.append(Doc);

	for (int i = 0; i < 4; ++i)
	{
		auto I = new QStandardItem("0");
		I->setData(jID, Qt::UserRole);
		Row.append(I);
	}

	model->appendRow(Row);
}

void LockWidget::removeDocument(int Index)
{
	QModelIndex Item;

	for (int i = 0, N = model->rowCount(); i < N; ++i)
	{
		const auto CP = model->index(i, 0);

		if (model->data(CP, Qt::UserRole).toInt() == Index)
		{
			Item = CP; i = model->rowCount(CP);
		}
	}

	if (Item.isValid()) model->removeRow(Item.row());
}

void LockWidget::recalcChanges(int Index, int Add, int Del, int Mod, int Err)
{
	int Row = -1;

	for (int i = 0, N = model->rowCount(); i < N; ++i)
	{
		const auto CP = model->index(i, 0);

		if (model->data(CP, Qt::UserRole).toInt() == Index)
		{
			Row = i; i = model->rowCount(CP);
		}
	}

	if (Row == -1) return;

	model->setData(model->index(Row, 1), Add);
	model->setData(model->index(Row, 2), Del);
	model->setData(model->index(Row, 3), Mod);
	model->setData(model->index(Row, 4), Err);
}

void LockWidget::itemSelected(const QModelIndex& Index)
{
	if (!Index.isValid()) return;

	const auto Data = model->index(Index.row(), 0, Index.parent());

	emit onDocSelected(model->data(Data, Qt::UserRole).toInt());
}
