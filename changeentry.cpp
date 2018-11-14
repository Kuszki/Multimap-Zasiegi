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

#include "changeentry.hpp"
#include "ui_changeentry.h"

ChangeEntry::ChangeEntry(const QVariantMap& Data, bool Lock, QWidget* Parent) :
QWidget(Parent), ui(new Ui::ChangeEntry), Locked(Lock), Status(0)
{
	ui->setupUi(this);

	setData(Data);
	setLocked(Lock);
}

ChangeEntry::~ChangeEntry(void)
{
	delete ui;
}

QVariantMap ChangeEntry::getOrigin(void) const
{
	return Origin;
}

QVariantMap ChangeEntry::getData(void) const
{
	QStringListModel* bModel = dynamic_cast<QStringListModel*>(ui->beforeView->model());
	QStringListModel* aModel = dynamic_cast<QStringListModel*>(ui->afterView->model());

	return
	{
		{ "uid", UID == 0 ? QVariant() : UID },
		{ "did", DID },
		{ "area", ui->areaEdit->text() },
		{ "sheet", ui->sheetEdit->text() },
		{ "before", bModel->stringList() },
		{ "after", aModel->stringList() },
		{ "status", Status }
	};
}

void ChangeEntry::setData(const QVariantMap& Data)
{
	UID = Data.value("uid").toInt();
	DID = Data.value("did").toInt();
	Origin = Data;

	ui->areaEdit->setText(Data.value("area").toString());
	ui->sheetEdit->setText(Data.value("sheet").toString());

	ui->beforeView->setModel(new QStringListModel(Data.value("before").toStringList(), this));
	ui->afterView->setModel(new QStringListModel(Data.value("after").toStringList(), this));

	updateStatus();
}

bool ChangeEntry::isChanged(void) const
{
	auto Now = getData();
	auto Org = Origin;

	Now.remove("status");
	Org.remove("status");

	return Now != Org;
}

bool ChangeEntry::isValid(void) const
{
	const auto Data = getData();

	return
	(
		!Data.value("area").toString().isEmpty() &&
		!Data.value("sheet").toString().isEmpty() &&
		!Data.value("before").toStringList().isEmpty() &&
		!Data.value("after").toStringList().isEmpty()
	);
}

bool ChangeEntry::isLocked(void) const
{
	return Locked;
}

void ChangeEntry::setLocked(bool Lock)
{
	ui->addLeftButton->setEnabled(!Lock);
	ui->addRightButton->setEnabled(!Lock);
	ui->delLeftButton->setEnabled(!Lock);
	ui->delRightButton->setEnabled(!Lock);

	ui->areaEdit->setReadOnly(Lock);
	ui->sheetEdit->setReadOnly(Lock);

	Locked = Lock;
}

void ChangeEntry::lock(void)
{
	setLocked(true);
}

void ChangeEntry::unlock(void)
{
	setLocked(false);
}

void ChangeEntry::addRightClicked(void)
{
	QStringListModel* aModel = dynamic_cast<QStringListModel*>(ui->afterView->model());

	const QStringList Current = aModel->stringList();
	const QStringList New = ui->commonEdit->text().split(QRegExp("[\\s,;.]+"),
											   QString::SkipEmptyParts);

	QStringList Now = Current + New;
	Now.removeDuplicates();

	aModel->setStringList(Now);

	ui->commonEdit->clear();
}

void ChangeEntry::addLeftClicked(void)
{
	QStringListModel* bModel = dynamic_cast<QStringListModel*>(ui->beforeView->model());

	const QStringList Current = bModel->stringList();
	const QStringList New = ui->commonEdit->text().split(QRegExp("[\\s,;.]+"),
											   QString::SkipEmptyParts);

	QStringList Now = Current + New;
	Now.removeDuplicates();

	bModel->setStringList(Now);

	ui->commonEdit->clear();
}

void ChangeEntry::delRightClicked(void)
{
	QStringListModel* aModel = dynamic_cast<QStringListModel*>(ui->afterView->model());

	aModel->removeRow(ui->afterView->selectionModel()->currentIndex().row());
}

void ChangeEntry::delLeftClicked(void)
{
	QStringListModel* bModel = dynamic_cast<QStringListModel*>(ui->beforeView->model());

	bModel->removeRow(ui->beforeView->selectionModel()->currentIndex().row());
}

void ChangeEntry::updateStatus(void)
{
	const bool New = Origin.value("uid").isNull();
	const bool Ok = isValid();
	const bool Ch = isChanged();

	auto d = getData();

	if (!Ok) Status = -1;
	else if (New) Status = 1;
	else if (Ch) Status = 2;
	else Status = 0;

	emit onStatusUpdate(Status);
}
