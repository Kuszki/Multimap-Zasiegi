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

QList<QVariantMap> ChangeWidget::getChanges(int Index) const
{
	if (Index == Currentindex) return getChanges();
	else return Unsaved.value(Index);
}

QList<QVariantMap> ChangeWidget::getChanges(void) const
{
	QList<QVariantMap> Changes;

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		const auto W = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		const auto Data = W->getData();

		if (Data.value("status").toInt()) Changes.append(Data);
	}

	return Changes;
}

QIcon ChangeWidget::getIcon(int Status) const
{
	switch (Status)
	{
		case -1: return QIcon::fromTheme("dialog-error");

		case 1: return QIcon::fromTheme("list-add");
		case 2: return QIcon::fromTheme("tools-check-spelling");
		case 3: return QIcon::fromTheme("list-remove");

		default: return QIcon();
	}
}

void ChangeWidget::discardChanged(int Index)
{
	Unsaved.remove(Index);

	if (Index == Currentindex)
	{
		while (ui->tabWidget->count())
		{
			ui->tabWidget->widget(0)->deleteLater();
			ui->tabWidget->removeTab(0);
		}

		setDocIndex(Index, true);
	}

	emit onChangesUpdate(Index, 0, 0, 0, 0);
}

void ChangeWidget::saveChanges(int Index)
{
	QList<QVariantMap> Current;

	for (const auto& List : getChanges(Index))
	{
		if (List.value("status").toInt() == -1)
		{
			Current.append(List);
		}
	}

	if (Current.isEmpty()) Unsaved.remove(Index);
	else Unsaved[Index] = Current;

	if (Index == Currentindex)
	{
		while (ui->tabWidget->count())
		{
			ui->tabWidget->widget(0)->deleteLater();
			ui->tabWidget->removeTab(0);
		}

		setDocIndex(Index, false, false);
	}

	emit onChangesUpdate(Index, 0, 0, 0, Current.size());
}

bool ChangeWidget::isLocked(void) const
{
	return Locked;
}

void ChangeWidget::updateStatus(void)
{
	int Add(0), Del(0), Mod(0), Err(0);

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		const int Status = Widget->getData().value("status").toInt();

		ui->tabWidget->setTabIcon(i, getIcon(Status));

		switch (Widget->getData().value("status").toInt())
		{
			case -1: ++Err; break;
			case 1: ++Add; break;
			case 2: ++Mod; break;
			case 3: ++Del; break;
		}
	}

	emit onChangesUpdate(Currentindex, Add, Del, Mod, Err);
}

void ChangeWidget::setDocIndex(int Index, bool Lock, bool Clear)
{
	QList<QVariantMap> Current = Clear ? getChanges() : Unsaved.value(Index);

	if (Current.isEmpty()) Unsaved.remove(Currentindex);
	else Unsaved[Currentindex] = Current;

	while (ui->tabWidget->count())
	{
		ui->tabWidget->widget(0)->deleteLater();
		ui->tabWidget->removeTab(0);
	}

	Currentindex = Index;
	Locked = Lock;

	if (!Currentindex) return;
	else Current = Unsaved.value(Index);
	unsigned ID = 0;

	QSqlQuery Query(Database);
	Query.prepare("SELECT id, arkusz, obreb, stare, nowe, uwagi "
			    "FROM zmiany WHERE dokument = ? "
			    "ORDER BY id ASC");
	Query.addBindValue(Index);

	if (Query.exec()) while (Query.next())
	{
		const QVariantMap Origin =
		{
			{ "uid", Query.value(0) },
			{ "did", Index },
			{ "sheet", Query.value(1) },
			{ "area", Query.value(2) },
			{ "before", Query.value(3).toString().split(';') },
			{ "after", Query.value(4).toString().split(';') },
			{ "comment", Query.value(5).toString() },
			{ "status", 0 }
		};

		ChangeEntry* Widget = new ChangeEntry(Origin, Database, Lock, this);
		ui->tabWidget->addTab(Widget, QString("%1").arg(++ID));

		connect(Widget, &ChangeEntry::onStatusUpdate, this, &ChangeWidget::updateStatus);
	}

	for (const auto& Change : Current)
	{
		int Found = -1;

		if (Change.value("uid").toInt())
			for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
			{
				auto W = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
				if (W->getData().value("uid") == Change.value("uid")) Found = i;
			}

		if (Found == -1)
		{
			ChangeEntry* Widget = new ChangeEntry(Change, Database, Lock, this);
			ui->tabWidget->addTab(Widget, getIcon(Change.value("status").toInt()), QString("%1").arg(++ID));

			connect(Widget, &ChangeEntry::onStatusUpdate, this, &ChangeWidget::updateStatus);
		}
		else
		{
			ChangeEntry* Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(Found));
			ui->tabWidget->setTabIcon(Found, getIcon(Change.value("status").toInt()));
			Widget->setData(Change);
		}
	}
}

void ChangeWidget::setLocked(bool Lock)
{
	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		ChangeEntry* Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		Widget->setLocked(Lock || Widget->getData().value("status").toInt() == 3);
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
	const QVariantMap Change =
	{
		{ "did", Currentindex },
		{ "status", 1 }
	};

	ChangeEntry* Widget = new ChangeEntry(Change, Database, false, this);
	ui->tabWidget->addTab(Widget, getIcon(1), QString("%1").arg(ui->tabWidget->count() + 1));

	connect(Widget, &ChangeEntry::onStatusUpdate, this, &ChangeWidget::updateStatus);
}

void ChangeWidget::removeChange(void)
{
	const int Index = ui->tabWidget->currentIndex();
	auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(Index));

	if (!Widget || Index == -1) return;

	QVariantMap Data = Widget->getOrigin();
	QVariantList Current;

	if (Data.value("uid").toInt())
	{
		Data["status"] = 3;
		Widget->setData(Data);
	}
	else
	{
		ui->tabWidget->removeTab(Index);
		Widget->deleteLater();
	}

	updateStatus();
}

void ChangeWidget::undoChange(void)
{
	const int Index = ui->tabWidget->currentIndex();
	auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(Index));

	if (!Widget || Index == -1) return;

	QVariantMap Data = Widget->getOrigin();
	QVariantList Current;

	if (Data.value("uid").toInt())
	{
		Data["status"] = 0;
		Widget->setData(Data);
		Widget->unlock();
	}
	else
	{
		ui->tabWidget->removeTab(Index);
		Widget->deleteLater();
	}

	updateStatus();
}
