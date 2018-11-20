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

#include "jobwidget.hpp"
#include "ui_jobwidget.h"

JobWidget::JobWidget(QSqlDatabase& Db, QWidget* Parent)
: QWidget(Parent), ui(new Ui::JobWidget)
{
	ui->setupUi(this);

	model = new SqlModel(this, Db);
	model->setTable("operaty");
	model->setEditStrategy(QSqlTableModel::OnRowChange);
	model->setEditable({ 2, 3 });

	model->setHeaderData(0, Qt::Horizontal, tr("ID"));
	model->setHeaderData(1, Qt::Horizontal, tr("P Number"));
	model->setHeaderData(2, Qt::Horizontal, tr("KERG Number"));
	model->setHeaderData(3, Qt::Horizontal, tr("Type"));

	model->setRelation(3, QSqlRelation("rodzajeopr", "id", "nazwa"));

	model->select();

	ui->tableView->setModel(model);
	ui->tableView->hideColumn(0);

	ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

	connect(ui->tableView->selectionModel(),
		   &QItemSelectionModel::currentRowChanged,
		   this, &JobWidget::selectionChanged);
}

JobWidget::~JobWidget(void)
{
	delete ui;
}

void JobWidget::setVisibleHeaders(const QVariantList& List)
{
	for (int i = 0; i < model->columnCount(); ++i)
	{
		ui->tableView->setColumnHidden(i, !List.isEmpty() && !List.contains(i));
	}
}

void JobWidget::refreshClicked(void)
{
	const QString Esc = ui->searchEdit->text().replace("'", "\\'");

	model->setFilter(QString("numer LIKE '%%1%' OR kerg LIKE '%%1%'").arg(Esc));
	ui->tableView->selectionModel()->clearSelection();

	emit onIndexChange(0);
}

void JobWidget::selectionChanged(const QModelIndex& Current)
{
	emit onIndexChange(model->getUid(Current).toInt());
}
