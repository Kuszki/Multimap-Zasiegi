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

#include "historywidget.hpp"
#include "ui_historywidget.h"

HistoryWidget::HistoryWidget(QSqlDatabase& Db, QWidget* Parent)
: QWidget(Parent), ui(new Ui::HistoryWidget), Database(Db)
{
	ui->setupUi(this);

	ui->treeView->setModel(new QStandardItemModel(this));

	refreshClicked();
}

HistoryWidget::~HistoryWidget(void)
{
	delete ui;
}

void HistoryWidget::loadByDate(void)
{
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(ui->treeView->model());
	QSqlQuery Query(Database);

	model->clear();
	model->setColumnCount(4);
	model->setHorizontalHeaderLabels(
	{
		tr("Date"), tr("Job"), tr("Documents"), tr("Changes")
	});

	QMap<QDate, QStandardItem*> Map;
	int Count(0);

	Query.prepare("SELECT DATE(d.data), o.numer, COUNT(DISTINCT d.id), COUNT(z.id) FROM dokumenty d "
			    "INNER JOIN operaty o ON d.operat = o.id "
			    "INNER JOIN zmiany z ON z.dokument = d.id "
			    "WHERE d.data IS NOT NULL and d.operator = ? "
			    "GROUP BY DATE(d.data), o.numer "
			    "ORDER BY 1");

	Query.addBindValue(AppCommon::getCurrentUser());

	if (Query.exec()) while (Query.next())
	{
		const QDate Date = Query.value(0).toDate();

		if (!Map.contains(Date))
		{
			Map.insert(Date, new QStandardItem(Date.toString()));
		}

		QStandardItem* Item = Map[Date];

		Item->appendRow(
		{
			new QStandardItem(),
			new QStandardItem(Query.value(1).toString()),
			new QStandardItem(Query.value(2).toString()),
			new QStandardItem(Query.value(3).toString())
		});

		Count += Query.value(2).toInt();
	}

	for (auto Item : Map) model->appendRow(Item);

	ui->doneCount->setText(QString::number(Count));
	ui->avgCount->setText(Map.isEmpty() ? "" : QString::number(double(Count) / double(Map.size()), 'f', 1));
}

void HistoryWidget::loadByJob(void)
{
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(ui->treeView->model());
	QSqlQuery Query(Database);

	model->clear();
	model->setColumnCount(3);
	model->setHorizontalHeaderLabels(
	{
		tr("Job"), tr("Documents"), tr("Changes")
	});

	QMap<QString, QStandardItem*> Map;
	int Count(0); QSet<QDate> Set;

	Query.prepare("SELECT DATE(d.data), o.numer, COUNT(DISTINCT d.id), COUNT(z.id) FROM dokumenty d "
			    "INNER JOIN operaty o ON d.operat = o.id "
			    "INNER JOIN zmiany z ON z.dokument = d.id "
			    "WHERE d.data IS NOT NULL and d.operator = ? "
			    "GROUP BY DATE(d.data), o.numer "
			    "ORDER BY 1");

	Query.addBindValue(AppCommon::getCurrentUser());

	if (Query.exec()) while (Query.next())
	{
		const QString Date = Query.value(1).toString();

		if (!Map.contains(Date))
		{
			Map.insert(Date, new QStandardItem(Date));
		}

		QStandardItem* Item = Map[Date];

		Item->appendRow(
		{
			new QStandardItem(),
			new QStandardItem(Query.value(2).toString()),
			new QStandardItem(Query.value(3).toString())
		});

		Count += Query.value(2).toInt();
		Set.insert(Query.value(0).toDate());
	}

	for (auto Item : Map) model->appendRow(Item);

	ui->doneCount->setText(QString::number(Count));
	ui->avgCount->setText(Set.isEmpty() ? "" : QString::number(double(Count) / double(Set.size()), 'f', 1));
}

void HistoryWidget::refreshClicked(void)
{
	if (ui->comboBox->currentIndex() == 0) loadByJob();
	else if (ui->comboBox->currentIndex() == 1) loadByDate();
}
