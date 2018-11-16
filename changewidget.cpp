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
	QList<QVariantMap> Changes;

	for (const auto& Change : Unsaved.value(Index))
	{
		Changes.append(Change.toMap());
	}

	return Changes;
}

void ChangeWidget::discardChanged(int Index)
{
	Unsaved.remove(Index);

	if (Index == Currentindex) setDocIndex(Index, true);

	emit onChangesUpdate(Index, QVariantList());
}

void ChangeWidget::saveChanges(int Index)
{
	QVariantList Current;

	for (const auto& List : Unsaved.value(Index))
	{
		if (List.toMap().value("status") == -1)
		{
			Current.append(List);
		}
	}

	if (Current.isEmpty()) Unsaved.remove(Index);
	else Unsaved[Index] = Current;

	if (Index == Currentindex) setDocIndex(Index, false);

	emit onChangesUpdate(Index, Current);
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
		const auto Data = Widget->getData();

		if (Widget == sender()) ui->tabWidget->setTabIcon(i, Icon);
		if (Status != 0) Current.append(Data);
	}

	if (Current.size())	Unsaved[Currentindex] = Current;
	else Unsaved.remove(Currentindex);

	emit onChangesUpdate(Currentindex, Current);
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
	auto Widget = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(Index));

	if (!Widget || Index == -1) return;

	QVariantMap Data = Widget->getOrigin();
	QVariantList Current;

	if (Data.value("uid").toInt())
	{
		Data["status"] = 3;

		Widget->blockSignals(true);
		Widget->setData(Data);
		Widget->lock();
		Widget->blockSignals(false);

		ui->tabWidget->setTabIcon(Index, QIcon::fromTheme("edit-delete"));
	}
	else
	{
		ui->tabWidget->removeTab(Index);
		Widget->deleteLater();
	}

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		const auto W = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		const auto D = W->getData();
		const int Status = D.value("status").toInt();

		if (Status != 0) Current.append(D);
	}

	if (Current.size())	Unsaved[Currentindex] = Current;
	else Unsaved.remove(Currentindex);

	emit onChangesUpdate(Currentindex, Current);
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

		Widget->blockSignals(true);
		Widget->setData(Data);
		Widget->unlock();
		Widget->blockSignals(false);

		ui->tabWidget->setTabIcon(Index, QIcon());
	}
	else
	{
		ui->tabWidget->removeTab(Index);
		Widget->deleteLater();
	}

	for (int i = 0, N = ui->tabWidget->count(); i < N; ++i)
	{
		const auto W = dynamic_cast<ChangeEntry*>(ui->tabWidget->widget(i));
		const auto D = W->getData();
		const int Status = D.value("status").toInt();

		if (Status != 0) Current.append(D);
	}

	if (Current.size())	Unsaved[Currentindex] = Current;
	else Unsaved.remove(Currentindex);

	emit onChangesUpdate(Currentindex, Current);
}
