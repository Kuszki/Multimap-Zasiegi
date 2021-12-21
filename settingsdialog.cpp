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

#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QSqlDatabase& Db, const QVariantHash& Op, QWidget* Parent)
: QDialog(Parent), ui(new Ui::SettingsDialog)
{
	QSqlQuery Query("SELECT id, nazwa FROM rodzajedok ORDER BY nazwa", Db);

	static const QHash<QString, int> jobFields =
	{
		{ tr("P Number"), 1 },
		{ tr("Type"), 2 },
	};

	static const QHash<QString, int> docFields =
	{
		{ tr("Name"), 1 },
		{ tr("Job"), 2 },
		{ tr("Type"), 3 },
		{ tr("Worker"), 4 },
		{ tr("Finished"), 5 },
		{ tr("Path"), 6 }
	};

	ui->setupUi(this);

	ui->typesLayout->setAlignment(Qt::AlignTop);
	ui->docLayout->setAlignment(Qt::AlignTop);
	ui->jobLayout->setAlignment(Qt::AlignTop);

	ui->countSpin->setValue(Op.value("Count", 1).toInt());

	while (Query.next())
	{
		QCheckBox* W = new QCheckBox(this);
		W->setText(Query.value(1).toString());
		W->setProperty("id", Query.value(0));
		W->setChecked(Op.value("Types").toList().contains(Query.value(0)));

		ui->typesLayout->addWidget(W);
	}

	for (auto i = jobFields.constBegin(); i != jobFields.constEnd(); ++i)
	{
		QCheckBox* W = new QCheckBox(this);
		W->setText(i.key());
		W->setProperty("id", i.value());
		W->setChecked(Op.value("Jobs").toList().contains(i.value()));

		ui->jobLayout->addWidget(W);
	}

	for (auto i = docFields.constBegin(); i != docFields.constEnd(); ++i)
	{
		QCheckBox* W = new QCheckBox(this);
		W->setText(i.key());
		W->setProperty("id", i.value());
		W->setChecked(Op.value("Docs").toList().contains(i.value()));

		ui->docLayout->addWidget(W);
	}
}

SettingsDialog::~SettingsDialog(void)
{
	delete ui;
}

QVariantHash SettingsDialog::getValues(void) const
{
	QVariantList Types, Jobs, Docs;

	for (int i = 0; i < ui->typesLayout->count(); ++i)
	{
		const auto W = dynamic_cast<QCheckBox*>(ui->typesLayout->itemAt(i)->widget());
		if (W->isChecked()) Types.append(W->property("id"));
	}

	for (int i = 0; i < ui->docLayout->count(); ++i)
	{
		const auto W = dynamic_cast<QCheckBox*>(ui->docLayout->itemAt(i)->widget());
		if (W->isChecked()) Docs.append(W->property("id"));
	}

	for (int i = 0; i < ui->jobLayout->count(); ++i)
	{
		const auto W = dynamic_cast<QCheckBox*>(ui->jobLayout->itemAt(i)->widget());
		if (W->isChecked()) Jobs.append(W->property("id"));
	}

	return
	{
		{ "Count", ui->countSpin->value() },
		{ "Types", Types },
		{ "Jobs", Jobs },
		{ "Docs", Docs }
	};
}

void SettingsDialog::accept(void)
{
	QDialog::accept();

	emit onSaveSettings(getValues());
}
