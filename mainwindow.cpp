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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* Parent)
: QMainWindow(Parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	updateImage(QString());

	Db = QSqlDatabase::addDatabase("QMYSQL");
	Db.setUserName("multimap");
	Db.setPassword("multimap");
	Db.setHostName("localhost");
	Db.setDatabaseName("zasiegi");
	Db.open();

	jwidget = new JobWidget(Db, this);
	jobs = new QDockWidget(tr("Jobs"), this);
	jobs->setWidget(jwidget);

	dwidget = new DocWidget(Db, this);
	docs = new QDockWidget(tr("Documents"), this);
	docs->setWidget(dwidget);

	addDockWidget(Qt::LeftDockWidgetArea, jobs);
	addDockWidget(Qt::LeftDockWidgetArea, docs);

	connect(jwidget, &JobWidget::onIndexChange,
		   dwidget, &DocWidget::setJobIndex);

	connect(dwidget, &DocWidget::onPathChange,
		   this, &MainWindow::updateImage);
}

MainWindow::~MainWindow(void)
{
	delete ui;
}

void MainWindow::aboutClicked(void)
{
	AboutDialog* Dialog = new AboutDialog(this);

	connect(Dialog, &AboutDialog::accepted,
		   Dialog, &AboutDialog::deleteLater);

	connect(Dialog, &AboutDialog::rejected,
		   Dialog, &AboutDialog::deleteLater);

	Dialog->open();
}

void MainWindow::scanClicked(void)
{
	IndexDialog* Dialog = new IndexDialog(this);

	connect(Dialog, &IndexDialog::accepted,
		   Dialog, &IndexDialog::deleteLater);

	connect(Dialog, &IndexDialog::rejected,
		   Dialog, &IndexDialog::deleteLater);

	connect(Dialog, &IndexDialog::onRefresh,
		   this, &MainWindow::scanDirectory);

	Dialog->open();
}

void MainWindow::rolesClicked(void)
{
	RoleDialog* Dialog = new RoleDialog(this);

	connect(Dialog, &RoleDialog::accepted,
		   Dialog, &RoleDialog::deleteLater);

	connect(Dialog, &RoleDialog::rejected,
		   Dialog, &RoleDialog::deleteLater);

	connect(Dialog, &RoleDialog::onRefresh,
		   this, &MainWindow::updateRoles);

	Dialog->open();
}

void MainWindow::zoomInClicked(void)
{
	ui->label->setPixmap(Image.scaledToHeight(int((Scale += 0.1) * Image.height())));
}

void MainWindow::zoomOutClicked(void)
{
	ui->label->setPixmap(Image.scaledToHeight(int((Scale -= 0.1) * Image.height())));
}

void MainWindow::zoomOrgClicked(void)
{
	ui->label->setPixmap(Image);
	Scale = 1.0;
}

void MainWindow::zoomFitClicked(void)
{
	const auto Scaled = Image.scaled(ui->scrollArea->size(), Qt::KeepAspectRatio);
	Scale = double(Scaled.height()) / double(Image.height());

	ui->label->setPixmap(Scaled);
}

void MainWindow::updateImage(const QString& Path)
{
	const bool Img = QFile(Path).exists();

	if (Img)
	{
		ui->label->clear();
		Image = QPixmap(Path);
		zoomFitClicked();
	}
	else
	{
		ui->label->clear();
		Image = QPixmap();
		ui->label->setText(tr("Select document"));
	}

	ui->actionFit->setEnabled(Img);
	ui->actionOrg->setEnabled(Img);
	ui->actionZoomin->setEnabled(Img);
	ui->actionZoomout->setEnabled(Img);
}

void MainWindow::scanDirectory(const QString& Dir, int Mode, bool Rec)
{
	const auto Flag = Rec ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
	const QStringList Filter = { "*.jpg", "*.jpeg"};

	QRegularExpression Expr("(\\w)\\.(\\d+)\\.(\\d+)\\.(\\d+)");

	QHash<QString, QStringList> Docs;
	QHash<QString, int> Jobs;

	QSqlQuery Query("SELECT id, numer FROM operaty", Db);

	while (Query.next())
	{
		Jobs.insert(Query.value(1).toString(), Query.value(0).toInt());
	}

	if (Mode == 2)
	{
		QDirIterator Iterator(Dir, Filter, QDir::Files, Flag);

		while (Iterator.hasNext())
		{
			const QString Path = Iterator.next();
			const QString Name = QFileInfo(Path).fileName();

			const auto Match = Expr.match(Name);

			if (Match.isValid())
			{
				Docs[Match.captured()].append(Path);
			}
		}
	}
	else if (Mode == 1)
	{
		QDirIterator Iterator(Dir, QDir::Dirs | QDir::NoDotAndDotDot);

		while (Iterator.hasNext())
		{
			const QString Path = Iterator.next();
			const QString Name = QFileInfo(Path).fileName();

			const auto Match = Expr.match(Name);

			if (Match.isValid())
			{
				QDirIterator Iterator(Path, Filter, QDir::Files);

				while (Iterator.hasNext())
				{
					Docs[Match.captured()].append(Iterator.next());
				}
			}
		}
	}
	else if (Mode == 0)
	{
		QDirIterator Iterator(Dir, QDir::Dirs | QDir::NoDotAndDotDot);

		while (Iterator.hasNext())
		{
			const QString Path = Iterator.next();
			const QString Name = QFileInfo(Path).fileName();

			if (QRegExp("^\\d+$").indexIn(Name) != -1)
			{
				QDirIterator Iterator(Path, QDir::Dirs | QDir::NoDotAndDotDot);

				while (Iterator.hasNext())
				{
					const QString Path = Iterator.next();
					const QString Name = QFileInfo(Path).fileName();

					const auto Match = Expr.match(Name);

					if (Match.isValid())
					{
						QDirIterator Iterator(Path, Filter, QDir::Files);

						while (Iterator.hasNext())
						{
							Docs[Match.captured()].append(Iterator.next());
						}
					}
				}
			}
		}
	}

	QtConcurrent::map(Docs, [] (auto& Data) -> void { Data.sort(); });

	Query.prepare("INSERT INTO operaty (numer) VALUES (?)");

	for (const auto& Key : Docs.keys()) if (!Jobs.contains(Key))
	{
		Query.addBindValue(Key);
		Query.exec();

		Jobs.insert(Key, Query.lastInsertId().toInt());
	}

	Query.prepare("INSERT INTO dokumenty (nazwa, sciezka, operat) VALUES (?, ?, ?) "
			    "ON DUPLICATE KEY UPDATE nazwa = ?, operat = ?");

	for (auto i = Docs.constBegin(); i != Docs.constEnd(); ++i)
	{
		const auto ID = Jobs.value(i.key());

		for (const auto& Path : i.value())
		{
			Query.addBindValue(QFileInfo(Path).fileName());
			Query.addBindValue(Path);
			Query.addBindValue(ID);
			Query.addBindValue(QFileInfo(Path).fileName());
			Query.addBindValue(ID);
			Query.exec();
		}
	}
}

void MainWindow::updateRoles(const QString& Path, bool Addnew)
{
	QSqlQuery Query("SELECT id, nazwa FROM rodzajedok", Db);

	QHash<QString, QString> Data;
	QHash<QString, int> Roles;
	QSet<QString> Newroles;

	while (Query.next())
	{
		Roles.insert(Query.value(1).toString(),
				   Query.value(0).toInt());
	}

	QFile Input(Path);
	QTextStream Stream(&Input);
	Input.open(QFile::Text | QFile::ReadOnly);

	while (!Stream.atEnd())
	{
		const QStringList Row = Stream.readLine().split('\t');
		if (Row.size() != 2) continue;

		Data.insert('%' + Row.value(0).replace("'", "\\'"), Row[1]);

		if (!Roles.contains(Row[1]))
		{
			Newroles.insert(Row[1]);
		}
	}

	Query.prepare("INSERT INTO rodzajedok (nazwa) VALUES (?)");

	if (Addnew) for (const auto& Role : Newroles)
	{
		Query.addBindValue(Role);
		Query.exec();

		Roles.insert(Role, Query.lastInsertId().toInt());
	}

	Query.prepare("UPDATE dokumenty SET rodzaj = ? WHERE sciezka LIKE ?");

	for (auto i = Data.constBegin(); i != Data.constEnd(); ++i) if (Roles.contains(i.value()))
	{
		Query.addBindValue(Roles.value(i.value()));
		Query.addBindValue(i.key());
		Query.exec();
	}
}
