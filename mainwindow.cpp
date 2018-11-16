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

	Db = QSqlDatabase::addDatabase("QMYSQL");
	Db.setUserName("multimap");
	Db.setPassword("multimap");
	Db.setHostName("localhost");
	Db.setDatabaseName("zasiegi");
	Db.open();

	cwidget = new ChangeWidget(Db, this);
	changes = new QDockWidget(tr("Changes"), this);
	changes->setObjectName("Changes");
	changes->setWidget(cwidget);

	lwidget = new LockWidget(this);
	locks = new QDockWidget(tr("Locked"), this);
	locks->setObjectName("Locked");
	locks->setWidget(lwidget);

	jwidget = new JobWidget(Db, this);
	jobs = new QDockWidget(tr("Jobs"), this);
	jobs->setObjectName("Jobs");
	jobs->setWidget(jwidget);

	dwidget = new DocWidget(Db, this);
	docs = new QDockWidget(tr("Documents"), this);
	docs->setObjectName("Documents");
	docs->setWidget(dwidget);

	addDockWidget(Qt::LeftDockWidgetArea, changes);
	addDockWidget(Qt::LeftDockWidgetArea, locks);
	addDockWidget(Qt::LeftDockWidgetArea, jobs);
	addDockWidget(Qt::LeftDockWidgetArea, docs);

	updateImage(QString());
	documentChanged(0);

	QSettings Settings("Multimap", "Zasiegi");

	Settings.beginGroup("Window");
	restoreGeometry(Settings.value("geometry").toByteArray());
	restoreState(Settings.value("state").toByteArray());
	Settings.endGroup();

	Settings.beginGroup("Reservations");
	Options.insert("Count", Settings.value("Count", 1).toInt());
	Options.insert("Types", Settings.value("Types").toList());
	Settings.endGroup();

	Settings.beginGroup("Headers");
	Options.insert("Jobs", Settings.value("Jobs").toList());
	Options.insert("Docs", Settings.value("Docs").toList());
	Settings.endGroup();

	QSqlQuery Query(Db);

	Query.prepare("SELECT d.nazwa, d.id, o.numer, o.id "
			    "FROM dokumenty d "
			    "INNER JOIN operaty o "
			    "ON d.operat = o.id "
			    "INNER JOIN blokady b "
			    "ON d.id = b.id "
			    "WHERE b.operator = ?");
	Query.addBindValue(getCurrentUser());

	if (Query.exec()) while (Query.next())
	{
		Locked.insert(Query.value(1).toInt());
		Queue.append(
		{
			Query.value(1).toInt(),
			Query.value(3).toInt()
		});

		lwidget->appendDocument(Query.value(0).toString(),
						    Query.value(1).toInt(),
						    Query.value(2).toString(),
						    Query.value(3).toInt());
	}

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());

	dwidget->setVisibleHeaders(Options.value("Docs").toList());
	jwidget->setVisibleHeaders(Options.value("Jobs").toList());

	connect(jwidget, &JobWidget::onIndexChange,
		   dwidget, &DocWidget::setJobIndex);

	connect(dwidget, &DocWidget::onIndexChange,
		   this, &MainWindow::documentChanged);

	connect(dwidget, &DocWidget::onPathChange,
		   this, &MainWindow::updateImage);

	connect(lwidget, &LockWidget::onDocSelected,
		   this, &MainWindow::focusDocument);

	connect(cwidget, &ChangeWidget::onChangesUpdate,
		   lwidget, &LockWidget::recalcChanges);

	connect(this, &MainWindow::onSaveChanges,
		   dwidget, &DocWidget::updateData);

	connect(this, &MainWindow::onSaveChanges,
		   cwidget, &ChangeWidget::saveChanges);
}

MainWindow::~MainWindow(void)
{
	QSettings Settings("Multimap", "Zasiegi");

	Settings.beginGroup("Window");
	Settings.setValue("state", saveState());
	Settings.setValue("geometry", saveGeometry());
	Settings.endGroup();

	Settings.beginGroup("Reservations");
	Settings.setValue("Count", Options.value("Count").toInt());
	Settings.setValue("Types", Options.value("Types").toList());
	Settings.endGroup();

	Settings.beginGroup("Headers");
	Settings.setValue("Jobs", Options.value("Jobs").toList());
	Settings.setValue("Docs", Options.value("Docs").toList());
	Settings.endGroup();


	delete ui;
}

QString MainWindow::getCurrentUser(void) const
{
	const QString U1 = qgetenv("USERNAME");
	const QString U2 = qgetenv("USER");
	const auto PID = QCoreApplication::applicationPid();

	return U1.isEmpty() ? (U2.isEmpty() ? QString::number(PID) : U2) : U1;
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

void MainWindow::settingsClicked(void)
{
	SettingsDialog* Dialog = new SettingsDialog(Db, Options, this);

	connect(Dialog, &SettingsDialog::accepted,
		   Dialog, &SettingsDialog::deleteLater);

	connect(Dialog, &SettingsDialog::rejected,
		   Dialog, &SettingsDialog::deleteLater);

	connect(Dialog, &SettingsDialog::onSaveSettings,
		   this, &MainWindow::saveSettings);

	Dialog->open();
}

void MainWindow::nextClicked(void)
{
	auto Index = Queue.takeFirst();

	if (CurrentDoc == Index.first && Queue.size())
	{
		Index = Queue.takeFirst();
	}

	const auto Job = dwidget->jobIndex();

	if (CurrentDoc) Queue.push_back({ CurrentDoc, Job });

	focusDocument(Index.first, Index.second);
}

void MainWindow::prevClicked(void)
{
	auto Index = Queue.takeLast();

	if (CurrentDoc == Index.first && Queue.size())
	{
		Index = Queue.takeLast();
	}

	const auto Job = dwidget->jobIndex();

	if (CurrentDoc) Queue.push_front({ CurrentDoc, Job });

	focusDocument(Index.first, Index.second);
}

void MainWindow::saveClicked(void)
{
	QSqlQuery delQuery(Db), updQuery(Db), addQuery(Db), docQuery(Db);

	delQuery.prepare("DELETE FROM zmiany WHERE id = ?");

	updQuery.prepare("UPDATE zmiany SET arkusz = ?, obreb = ?, stare = ?, nowe = ? WHERE id = ?");

	addQuery.prepare("INSERT INTO zmiany (dokument, arkusz, obreb, stare, nowe) VALUES (?, ?, ?, ?, ?)");

	docQuery.prepare("UPDATE dokumenty SET operator = ?, data = CURRENT_TIMESTAMP WHERE id = ?");

	for (const auto& Change : cwidget->getChanges(CurrentDoc)) switch (Change.value("status").toInt())
	{
		case 1:
			addQuery.addBindValue(Change.value("did"));
			addQuery.addBindValue(Change.value("sheet"));
			addQuery.addBindValue(Change.value("area"));
			addQuery.addBindValue(Change.value("before").toStringList().join(';'));
			addQuery.addBindValue(Change.value("after").toStringList().join(';'));

			addQuery.exec();
		break;
		case 2:
			updQuery.addBindValue(Change.value("sheet"));
			updQuery.addBindValue(Change.value("area"));
			updQuery.addBindValue(Change.value("before").toStringList().join(';'));
			updQuery.addBindValue(Change.value("after").toStringList().join(';'));
			updQuery.addBindValue(Change.value("uid"));

			updQuery.exec();
		break;
		case 3:
			delQuery.addBindValue(Change.value("uid"));

			delQuery.exec();
		break;
	}

	docQuery.addBindValue(getCurrentUser());
	docQuery.addBindValue(CurrentDoc);

	docQuery.exec();

	emit onSaveChanges(CurrentDoc);
}

void MainWindow::editClicked(void)
{
	QSqlQuery Query(Db);

	Query.prepare("INSERT INTO blokady (id, operator) "
			    "VALUES (?, ?)");

	Query.addBindValue(CurrentDoc);
	Query.addBindValue(getCurrentUser());

	if (Query.exec())
	{
		Query.prepare("SELECT d.nazwa, d.id, o.numer, o.id "
				    "FROM dokumenty d "
				    "INNER JOIN operaty o "
				    "ON d.operat = o.id "
				    "WHERE d.id = ?");

		Query.addBindValue(CurrentDoc);

		if (Query.exec() && Query.next())
		{
			Queue.append(qMakePair(Query.value(1).toInt(),
							   Query.value(3).toInt()));

			lwidget->appendDocument(Query.value(0).toString(),
							    Query.value(1).toInt(),
							    Query.value(2).toString(),
							    Query.value(3).toInt());
		}

		Locked.insert(CurrentDoc);
		documentChanged(CurrentDoc);
	}
}

void MainWindow::lockClicked(void)
{
	const QString Types = Options.value("Types").toStringList().join(',');
	const int Count = Options.value("Count", 1).toInt();
	const QString User = getCurrentUser();

	QVector<QString> dNames, jNames;
	QVector<int> dIDS, jIDS;
	int Size(0);

	QSqlQuery Query(QString("SELECT d.nazwa, d.id, o.numer, o.id FROM dokumenty d "
					    "INNER JOIN operaty o ON d.operat = o.id "
					    "WHERE d.rodzaj IN (%2) AND d.data IS NULL "
					    "AND d.id NOT IN (SELECT b.id FROM blokady b) "
					    "LIMIT %1")
				 .arg(Count).arg(Types), Db);

	while (Query.next())
	{
		dNames.append(Query.value(0).toString());
		jNames.append(Query.value(2).toString());
		dIDS.append(Query.value(1).toInt());
		jIDS.append(Query.value(3).toInt());

		++Size;
	}

	Query.prepare("INSERT INTO blokady (id, operator) "
			    "VALUES (?, ?)");

	for (int i = 0; i < Size; ++i)
	{
		Query.addBindValue(dIDS[i]);
		Query.addBindValue(User);

		if (Query.exec())
		{
			lwidget->appendDocument(dNames[i], dIDS[i],
							    jNames[i], jIDS[i]);

			Queue.append({ dIDS[i], jIDS[i] });
			Locked.insert(dIDS[i]);
		}
	}
}

void MainWindow::unlockClicked(void)
{
	QSqlQuery Query(Db);

	Query.prepare("DELETE FROM blokady WHERE id = ?");
	Query.addBindValue(CurrentDoc);
	Query.exec();

	Locked.remove(CurrentDoc);
	cwidget->discardChanged(CurrentDoc);

	ui->actionAddchange->setEnabled(false);
	ui->actionRemovechange->setEnabled(false);
	ui->actionUndochange->setEnabled(false);

	ui->actionLock->setEnabled(true);
	ui->actionSave->setEnabled(false);
	ui->actionUnlock->setEnabled(false);

	lwidget->removeDocument(CurrentDoc);
}

void MainWindow::zoomInClicked(void)
{
	ui->label->setPixmap(Image.scaledToHeight(int((Scale += 0.1) * Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::zoomOutClicked(void)
{
	ui->label->setPixmap(Image.scaledToHeight(int((Scale -= 0.1) * Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::zoomOrgClicked(void)
{
	ui->label->setPixmap(Image.transformed(QTransform().rotate(Rotation)));

	Scale = 1.0;
}

void MainWindow::zoomFitClicked(void)
{
	auto Img = Image.transformed(QTransform().rotate(Rotation))
			 .scaled(ui->scrollArea->size(), Qt::KeepAspectRatio);

	Scale = double(Img.height()) / double(Img.height());

	ui->label->setPixmap(Img);
}

void MainWindow::rotateLeftClicked(void)
{
	if ((Rotation -= 90) <= -360) Rotation += 360;

	ui->label->setPixmap(Image.scaledToHeight(int(Scale* Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::rotateRightClicked(void)
{
	if ((Rotation -= 90) <= -360) Rotation += 360;

	ui->label->setPixmap(Image.scaledToHeight(int(Scale* Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::changeAddClicked(void)
{
	cwidget->appendChange();
}

void MainWindow::changeDelClicked(void)
{
	cwidget->removeChange();
}

void MainWindow::changeUndoClicked(void)
{
	cwidget->undoChange();
}

void MainWindow::documentChanged(int Index)
{
	const bool Lock = Locked.contains(Index);

	ui->actionAddchange->setEnabled(Lock);
	ui->actionRemovechange->setEnabled(Lock);
	ui->actionUndochange->setEnabled(Lock);

	ui->actionLock->setEnabled(!Lock && Index);
	ui->actionSave->setEnabled(Lock);
	ui->actionUnlock->setEnabled(Lock);

	cwidget->setDocIndex(Index, !Lock);
	CurrentDoc = Index;
}

void MainWindow::focusDocument(int Document, int Job)
{
	dwidget->setJobIndex(Job);
	dwidget->setDocIndex(Document);
}

void MainWindow::updateImage(const QString& Path)
{
	const bool Img = QFile(Path).exists();

	if (Img)
	{
		Rotation = 0;

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
	ui->actionRotateleft->setEnabled(Img);
	ui->actionRotateright->setEnabled(Img);
}

void MainWindow::saveSettings(const QVariantMap& Settings)
{
	Options = Settings;

	dwidget->setVisibleHeaders(Options.value("Docs").toList());
	jwidget->setVisibleHeaders(Options.value("Jobs").toList());
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

			QApplication::processEvents();
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

			QApplication::processEvents();
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

			QApplication::processEvents();
		}
	}

	QtConcurrent::map(Docs, [] (auto& Data) -> void { Data.sort(); });

	Query.prepare("INSERT INTO operaty (numer) VALUES (?)");

	for (const auto& Key : Docs.keys()) if (!Jobs.contains(Key))
	{
		Query.addBindValue(Key);
		Query.exec();

		Jobs.insert(Key, Query.lastInsertId().toInt());

		QApplication::processEvents();
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

		QApplication::processEvents();
	}
}

void MainWindow::updateRoles(const QString& Path, bool Addnew)
{
	QSqlQuery Query("SELECT id, nazwa FROM rodzajedok", Db);

	QList<QPair<int, QString>> Documents;
	QList<QPair<int, int>> Updates;

	QHash<QString, QString> Data;
	QHash<QString, int> Roles;
	QSet<QString> Newroles;

	QMutex Locker;

	while (Query.next()) Roles.insert(
				Query.value(1).toString(),
				Query.value(0).toInt());

	QFile Input(Path);
	QTextStream Stream(&Input);
	Input.open(QFile::Text | QFile::ReadOnly);

	while (!Stream.atEnd())
	{
		const QStringList Row = Stream.readLine().split('\t');
		if (Row.size() != 2) continue;

		Data.insert(Row[0], Row[1]);

		if (!Roles.contains(Row[1]))
		{
			Newroles.insert(Row[1]);
		}

		QApplication::processEvents();
	}

	Query.prepare("INSERT INTO rodzajedok (nazwa) VALUES (?)");

	if (Addnew) for (const auto& Role : Newroles)
	{
		Query.addBindValue(Role);
		Query.exec();

		Roles.insert(Role, Query.lastInsertId().toInt());
	}

	Query.prepare("SELECT id, sciezka FROM dokumenty");

	if (Query.exec()) while (Query.next()) Documents.append(
	{
		Query.value(0).toInt(),
		Query.value(1).toString()
	});

	QtConcurrent::map(Documents, [&Data, &Roles, &Updates, &Locker] (QPair<int, QString>& Row) -> void
	{
		for (auto i = Data.constBegin(); i != Data.constEnd(); ++i)
		{
			if (Row.second.endsWith(i.key()))
			{
				const int Role = Roles.value(i.value(), 0);

				if (Role)
				{
					Locker.lock();
					Updates.append({ Row.first, Role });
					Locker.unlock();
				}
			}
		}
	});

	Query.prepare("UPDATE dokumenty SET rodzaj = ? WHERE id = ?");

	for (const auto& Row : Updates)
	{
		Query.addBindValue(Row.second);
		Query.addBindValue(Row.first);
		Query.exec();

		QApplication::processEvents();
	}
}
