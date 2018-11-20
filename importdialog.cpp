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

#include "importdialog.hpp"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::ImportDialog)
{
	ui->setupUi(this);
	pathChanged(QString());

	auto jItem = new QStandardItem(tr("Select jobs types filter"));
	jmodel = new QStandardItemModel(0, 1, this);
	jItem->setFlags(Qt::ItemIsEnabled);
	jmodel->appendRow(jItem);

	ui->jobCombo->setModel(jmodel);

	auto dItem = new QStandardItem(tr("Select documents types filter"));
	dmodel = new QStandardItemModel(0, 1, this);
	dItem->setFlags(Qt::ItemIsEnabled);
	dmodel->appendRow(dItem);

	ui->docCombo->setModel(dmodel);
}

ImportDialog::~ImportDialog(void)
{
	delete ui;
}

void ImportDialog::updateData(const QString& Path)
{
	setEnabled(false);

	QFile Input(Path);
	QTextStream Stream(&Input);
	Input.open(QFile::Text | QFile::ReadOnly);

	QSet<QString> jTypes, dTypes;

	while (!Stream.atEnd())
	{
		const QStringList Row = Stream.readLine().split('\t');
		if (Row.size() != 4) continue;

		if (!Row[2].isEmpty() && Row[2] != "-")
		{
			dTypes.insert(Row[2]);
			Docs.insert(Row[1], QStringList(
			{
				Row[0], Row[2]
			}));
		}

		if (!Row[3].isEmpty() && Row[3] != "-")
		{
			jTypes.insert(Row[3]);
			Jobs[Row[0]] = Row[3];
		}

		QApplication::processEvents();
	}

	while (dmodel->rowCount() > 1) dmodel->removeRow(1);
	while (jmodel->rowCount() > 1) jmodel->removeRow(1);

	for (const auto& Type : dTypes)
	{
		auto Item = new QStandardItem(Type);

		Item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		Item->setCheckState(Qt::Checked);

		dmodel->appendRow(Item);
	}

	for (const auto& Type : jTypes)
	{
		auto Item = new QStandardItem(Type);

		Item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		Item->setCheckState(Qt::Checked);

		jmodel->appendRow(Item);
	}

	setEnabled(true);
}

void ImportDialog::accept(void)
{
	QDialog::accept();

	QSet<QString> jTypes, dTypes;
	QVariantMap sDocs, sJobs;

	for (int i = 1; i < jmodel->rowCount(); ++i)
	{
		const auto Item = jmodel->item(i, 0);

		if (Item->checkState() == Qt::Checked)
		{
			jTypes.insert(Item->text());
		}
	}

	for (int i = 1; i < dmodel->rowCount(); ++i)
	{
		const auto Item = dmodel->item(i, 0);

		if (Item->checkState() == Qt::Checked)
		{
			dTypes.insert(Item->text());
		}
	}

	for (auto i = Docs.constBegin(); i != Docs.constEnd(); ++i)
	{
		if (dTypes.contains(i.value().toStringList().value(1)))
		{
			sDocs.insert(i.key(), i.value());
		}
	}

	for (auto i = Jobs.constBegin(); i != Jobs.constEnd(); ++i)
	{
		if (jTypes.contains(i.value().toString()))
		{
			sJobs.insert(i.key(), i.value());
		}
	}

	emit onLoadRequest(sJobs, sDocs);
}

void ImportDialog::openClicked(void)
{
	const QString Path = QFileDialog::getOpenFileName(this, tr("Load data"));

	if (!Path.isEmpty()) ui->pathEdit->setText(Path);
}

void ImportDialog::pathChanged(const QString& Path)
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!Path.isEmpty());

	if (!Path.isEmpty()) updateData(Path);
}
