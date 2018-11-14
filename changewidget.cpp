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

#include "changewidget.hpp"
#include "ui_changewidget.h"

ChangeWidget::ChangeWidget(QSqlDatabase& Db, QWidget* Parent)
: QWidget(Parent), ui(new Ui::ChangeWidget), Database(Db)
{
	ui->setupUi(this);
}

ChangeWidget::~ChangeWidget(void)
{
	delete ui;
}

QList<QVariantMap> ChangeWidget::getChanges(void) const
{
	QList<QVariantMap> Changes;

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		ChangeEntry* Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));

		if (Widget) Changes.append(Widget->getData());
	}

	return Changes;
}

void ChangeWidget::saveChanges(void)
{
	QSqlQuery Query(Database);
	QSet<int> Current;

	Query.prepare("SELECT id FROM zmiany WHERE dokument = ?");
	Query.addBindValue(Currentindex);

	if (Query.exec()) while (Query.next())
	{
		Current.insert(Query.value(0).toInt());
	}

	Query.prepare("INSTERT INTO zmiany (id, dokument, arkusz, obreb, stare, nowe) "
			    "VALUES (?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE "
			    "arkusz = ?, obreb = ?, stare = ?, nowe = ?");

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		ChangeEntry* Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		auto Change = Widget->getData();

		Current.remove(Change.value("uid").toInt());

		Query.addBindValue(Change.value("uid"));
		Query.addBindValue(Change.value("did"));
		Query.addBindValue(Change.value("sheet"));
		Query.addBindValue(Change.value("area"));
		Query.addBindValue(Change.value("before").toStringList().join(';'));
		Query.addBindValue(Change.value("after").toStringList().join(';'));
		Query.addBindValue(Change.value("sheet"));
		Query.addBindValue(Change.value("area"));
		Query.addBindValue(Change.value("before").toStringList().join(';'));
		Query.addBindValue(Change.value("after").toStringList().join(';'));

		Query.exec();

		Change["uid"] = Query.lastInsertId();

		ui->tabWidget->setTabIcon(i, QIcon());
		Widget->setData(Change);
	}

	Query.prepare("DELETE FROM zmiany WHERE id = ?");

	for (const auto& ID : Current)
	{
		Query.addBindValue(ID);
		Query.exec();
	}
}

bool ChangeWidget::isLocked(void) const
{
	return Locked;
}

void ChangeWidget::updateStatus(int Status)
{
	QVariantList Current;

	const auto Icon = Status == 2 ?
					   QIcon::fromTheme("tools-check-spelling") :
					   Status == 1 ?
						   QIcon::fromTheme("list-add") :
						   Status == 0 ?
							   QIcon() :
							   QIcon::fromTheme("error");

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));

		if (Widget == sender())
		{
			ui->tabWidget->setTabIcon(i, Icon);
		}

		Current.append(Widget->getData());
	}

	if (Current.size())	Unsaved[Currentindex] = Current;
	else Unsaved.remove(Currentindex);
}

void ChangeWidget::setDocIndex(int Index, bool Lock)
{
	QSqlQuery Query(Database);

	Currentindex = Index;
	Locked = Lock;

	unsigned ID = 0;

	Query.prepare("SELECT id, arkusz, obreb, stare, nowe "
			    "FROM zmiany WHERE dokument = ? "
			    "ORDER BY id ASC");

	Query.addBindValue(Index);

	while (ui->tabWidget->count())
	{
		ui->tabWidget->widget(0)->deleteLater();
		ui->tabWidget->removeTab(0);
	}

	if (Query.exec()) while (Query.next())
	{
		const auto Changes = Unsaved.value(Index);
		const auto UID = Query.value(0);
		int Exists = -1;

		for (int i = 0, N = Changes.size(); i < N; ++i)
		{
			if (Changes[i].toMap().value("uid") == UID) Exists = i;
		}

		QVariantMap Data = Exists == -1 ?
		QVariantMap(
		{
			{ "uid", UID },
			{ "did", Index },
			{ "sheet", Query.value(1) },
			{ "area", Query.value(2) },
			{ "before", Query.value(3).toString().split(';') },
			{ "after", Query.value(4).toString().split(';') },
			{ "status", 0 }
		}) : Changes[Exists].toMap();

		const auto Status = Data.value("status").toInt();

		if (Status != 3)
		{
			const auto Icon = Status == 2 ?
							   QIcon::fromTheme("tools-check-spelling") :
							   Status == 0 ?
								   QIcon() :
								   QIcon::fromTheme("error");

			ChangeEntry* Widget = new ChangeEntry(Data, Lock, this);

			ui->tabWidget->addTab(Widget, Icon, QString("%1").arg(++ID));

			connect(Widget, &ChangeEntry::onStatusUpdate,
				   this, &ChangeWidget::updateStatus);
		}
	}

	for (const auto& Change : Unsaved.value(Index))
	{
		const auto Data = Change.toMap();

		if (Data.value("uid").toInt() == 0)
		{
			const auto Status = Data.value("status").toInt();

			const auto Icon = Status == 1 ?
							   QIcon::fromTheme("list-add") :
							   QIcon::fromTheme("error");

			ChangeEntry* Widget = new ChangeEntry(Data, Lock, this);

			ui->tabWidget->addTab(Widget, Icon, QString("%1").arg(++ID));

			connect(Widget, &ChangeEntry::onStatusUpdate,
				   this, &ChangeWidget::updateStatus);
		}
	}
}

void ChangeWidget::setLocked(bool Lock)
{
	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		ChangeEntry* Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));

		if (Widget) Widget->setLocked(Lock);
	}

	Locked = Lock;
}

void ChangeWidget::lock(void)
{
	setLocked(true);
}

void ChangeWidget::unlock(void)
{
	setLocked(false);
}

void ChangeWidget::appendChange(void)
{
	ChangeEntry* Widget = new ChangeEntry({ { "did", Currentindex } }, false, this);

	ui->tabWidget->addTab(Widget, QIcon::fromTheme("list-add"),
					  QString("%1").arg(ui->tabWidget->count() + 1));

	connect(Widget, &ChangeEntry::onStatusUpdate,
		   this, &ChangeWidget::updateStatus);
}

void ChangeWidget::removeChange(void)
{
	const int Index = ui->tabWidget->currentIndex();

	QVariantList Current;
	QVariantMap Data;

	if (Index != -1)
	{
		Data = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(Index))->getOrigin();

		ui->tabWidget->widget(Index)->deleteLater();
		ui->tabWidget->removeTab(Index);

		Data["status"] = 3;
	}
	else return;

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));

		Current.append(Widget->getData());
	}

	if (Data.value("uid").toInt())
	{
		Current.append(Data);
	}

	if (Current.size())	Unsaved[Currentindex] = Current;
	else Unsaved.remove(Currentindex);
}
