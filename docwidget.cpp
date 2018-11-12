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

#include "docwidget.hpp"
#include "ui_docwidget.h"

DocWidget::DocWidget(QSqlDatabase& Db, QWidget* Parent)
: QWidget(Parent), ui(new Ui::DocWidget)
{
	ui->setupUi(this);

	model = new SqlModel(this, Db);
	model->setTable("dokumenty");
	model->setEditStrategy(QSqlRelationalTableModel::OnRowChange);
	model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
	model->setEditable({ 3 });

	model->setHeaderData(0, Qt::Horizontal, tr("ID"));
	model->setHeaderData(1, Qt::Horizontal, tr("Name"));
	model->setHeaderData(2, Qt::Horizontal, tr("Job"));
	model->setHeaderData(3, Qt::Horizontal, tr("Type"));
	model->setHeaderData(4, Qt::Horizontal, tr("Worker"));
	model->setHeaderData(5, Qt::Horizontal, tr("Finished"));
	model->setHeaderData(6, Qt::Horizontal, tr("Path"));

	model->setRelation(2, QSqlRelation("operaty", "id", "numer"));
	model->setRelation(3, QSqlRelation("rodzajedok", "id", "nazwa"));

	model->setFilter(QString("operat = 0"));
	model->select();

	ui->tableView->setModel(model);
	ui->tableView->hideColumn(0);
	ui->tableView->hideColumn(2);

	ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

	connect(ui->tableView->selectionModel(),
		   &QItemSelectionModel::currentRowChanged,
		   this, &DocWidget::selectionChanged);
}

DocWidget::~DocWidget(void)
{
	delete ui;
}

QString DocWidget::currentImage(void) const
{
	const auto Current = ui->tableView->selectionModel()->currentIndex();
	return model->data(model->index(Current.row(), 6)).toString();
}

void DocWidget::setJobIndex(int Index)
{
	model->setFilter(QString("operat = %1").arg(Index));
}

void DocWidget::selectionChanged(const QModelIndex& Current)
{
	emit onIndexChange(model->data(model->index(Current.row(), 0)).toInt());
	emit onPathChange(model->data(model->index(Current.row(), 6)).toString());
}
