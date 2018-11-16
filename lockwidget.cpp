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
		tr("Document"), tr("Added"), tr("Removed"), tr("Modified")
	});

	ui->treeView->setModel(model);

	connect(ui->treeView, &QTreeView::doubleClicked,
		   this, &LockWidget::itemSelected);
}

LockWidget::~LockWidget(void)
{
	delete ui;
}

void LockWidget::appendDocument(const QString& File, int dID, const QString& Job, int jID)
{
	QList<QStandardItem*> Row;
	QModelIndex Parent;

	auto* Doc = new QStandardItem(File);
	Doc->setData(dID, Qt::UserRole);
	Row.append(Doc);

	for (int i = 0; i < 3; ++i)
	{
		auto I = new QStandardItem("0");
		I->setData(dID, Qt::UserRole);
		Row.append(I);
	}

	for (int i = 0, N = model->rowCount(); i < N; ++i)
	{
		const auto Current = model->index(i, 0);

		if (model->data(Current, Qt::UserRole) == jID)
		{
			Parent = Current;
			i = N;
		}
	}

	if (!Parent.isValid())
	{
		auto Item = new QStandardItem(Job);
		Item->setData(jID, Qt::UserRole);

		model->appendRow(Item);
		Parent = Item->index();
	}

	model->itemFromIndex(Parent)->appendRow(Row);
}

void LockWidget::removeDocument(int Index)
{
	QModelIndex Parent, Item;

	for (int i = 0, N = model->rowCount(); i < N; ++i)
	{
		const auto CP = model->index(i, 0);

		for (int j = 0, M = model->rowCount(CP); j < M; ++j)
		{
			const auto CI = model->index(j, 0, CP);

			if (model->data(CI, Qt::UserRole).toInt() == Index)
			{
				Parent = CP; i = model->rowCount();
				Item = CI; j = model->rowCount(CP);
			}
		}
	}

	if (Item.isValid())
		model->removeRow(Item.row(), Parent);

	if (Parent.isValid() && !model->rowCount(Parent))
		model->removeRow(Parent.row());
}

void LockWidget::recalcChanges(int Index, int Add, int Del, int Mod)
{
	QModelIndex Parent;
	int Row = -1;

	for (int i = 0, N = model->rowCount(); i < N; ++i)
	{
		const auto Pind = model->index(i, 0);

		for (int j = 0, M = model->rowCount(Pind); j < M; ++j)
		{
			const auto Cind = model->index(j, 0, Pind);

			if (model->data(Cind, Qt::UserRole).toInt() == Index)
			{
				Parent = Pind;
				Row = j;

				j = model->item(i, 0)->rowCount();
				i = model->rowCount();
			}
		}
	}

	if (!Parent.isValid() || Row == -1) return;

	model->setData(model->index(Row, 1, Parent), Add);
	model->setData(model->index(Row, 2, Parent), Del);
	model->setData(model->index(Row, 3, Parent), Mod);
}

void LockWidget::itemSelected(const QModelIndex& Index)
{
	const auto Data = model->index(Index.row(), 0, Index.parent());

	if (Index.parent().isValid()) emit onDocSelected(
				model->data(Data, Qt::UserRole).toInt(),
				model->data(Index.parent(), Qt::UserRole).toInt());
}
