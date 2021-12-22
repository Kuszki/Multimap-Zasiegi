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
	model->setColumnCount(3);
	model->setHorizontalHeaderLabels(
	{
		tr("Date"), tr("Job"), tr("Changes")
	});

	QMap<QString, QStandardItem*> Map;
	int Count(0); QSet<QDate> Set;

	Query.prepare("SELECT o.numer, DATE(o.data), COUNT(z.id) FROM operaty o "
			    "INNER JOIN zmiany z ON z.dokument = o.id "
			    "WHERE o.data IS NOT NULL and o.operator = ? "
			    "GROUP BY o.numer, DATE(o.data) "
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
			new QStandardItem(Query.value(0).toString()),
			new QStandardItem(Query.value(2).toString())
		});

		Set.insert(Query.value(1).toDate());
		Count += 1;
	}
	else qDebug() << Query.lastError().text();

	for (auto Item : Map) model->appendRow(Item);

	ui->doneCount->setText(QString::number(Count));
	ui->avgCount->setText(Set.isEmpty() ? tr("na") : QString::number(double(Count) / double(Set.size()), 'f', 1));
}

void HistoryWidget::loadByJob(void)
{
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(ui->treeView->model());
	QSqlQuery Query(Database);

	model->clear();
	model->setColumnCount(3);
	model->setHorizontalHeaderLabels(
	{
		tr("Job"), tr("Date"), tr("Changes")
	});

	QMap<QString, QStandardItem*> Map;
	int Count(0); QSet<QDate> Set;

	Query.prepare("SELECT o.numer, DATE(o.data), COUNT(z.id) FROM operaty o "
			    "INNER JOIN zmiany z ON z.dokument = o.id "
			    "WHERE o.data IS NOT NULL and o.operator = ? "
			    "GROUP BY o.numer, DATE(o.data) "
			    "ORDER BY 1");

	Query.addBindValue(AppCommon::getCurrentUser());

	if (Query.exec()) while (Query.next())
	{
		const QString Date = Query.value(0).toString();

		if (!Map.contains(Date))
		{
			Map.insert(Date, new QStandardItem(Date));
		}

		QStandardItem* Item = Map[Date];

		Item->appendRow(
		{
			new QStandardItem(),
			new QStandardItem(Query.value(1).toString()),
			new QStandardItem(Query.value(2).toString())
		});

		Set.insert(Query.value(1).toDate());
		Count += 1;
	}

	for (auto Item : Map) model->appendRow(Item);

	ui->doneCount->setText(QString::number(Count));
	ui->avgCount->setText(Set.isEmpty() ? tr("na") : QString::number(double(Count) / double(Set.size()), 'f', 1));
}

void HistoryWidget::refreshClicked(void)
{
	if (ui->comboBox->currentIndex() == 0) loadByJob();
	else if (ui->comboBox->currentIndex() == 1) loadByDate();
}
