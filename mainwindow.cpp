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

	QSettings Settings("Multimap", "Zasiegi");

	Settings.beginGroup("Database");
	Options.insert("Count", Settings.value("Count", 1).toInt());
	Options.insert("Types", Settings.value("Types").toList());

	Db = QSqlDatabase::addDatabase("QMYSQL");
	Db.setUserName(Settings.value("user", "multimap").toString());
	Db.setPassword(Settings.value("password", "multimap").toString());
	Db.setHostName(Settings.value("server", "localhost").toString());
	Db.setDatabaseName(Settings.value("database", "zasiegi").toString());
	Db.open();

	Settings.endGroup();

	hwidget = new HistoryWidget(Db, this);
	history = new QDockWidget(tr("History"), this);
	history->setObjectName("History");
	history->setWidget(hwidget);

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

	addDockWidget(Qt::LeftDockWidgetArea, history);
	addDockWidget(Qt::LeftDockWidgetArea, changes);
	addDockWidget(Qt::LeftDockWidgetArea, locks);
	addDockWidget(Qt::LeftDockWidgetArea, jobs);
	addDockWidget(Qt::LeftDockWidgetArea, docs);

	updateImage(QString());
	documentChanged(0);

	Settings.beginGroup("Window");
	restoreGeometry(Settings.value("geometry").toByteArray());
	restoreState(Settings.value("state").toByteArray());
	Settings.endGroup();

	Settings.beginGroup("Reservations");
	Options.insert("Count", Settings.value("count", 1).toInt());
	Options.insert("Types", Settings.value("types").toList());
	Settings.endGroup();

	Settings.beginGroup("Headers");
	Options.insert("Jobs", Settings.value("jobs").toList());
	Options.insert("Docs", Settings.value("docs").toList());
	Settings.endGroup();

	QSqlQuery Query(Db);

	Query.prepare("SELECT d.nazwa, d.id, o.numer, o.id "
			    "FROM dokumenty d "
			    "INNER JOIN operaty o "
			    "ON d.operat = o.id "
			    "INNER JOIN blokady b "
			    "ON d.id = b.id "
			    "WHERE b.operator = ?");
	Query.addBindValue(AppCommon::getCurrentUser());

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
	Settings.setValue("count", Options.value("Count").toInt());
	Settings.setValue("types", Options.value("Types").toList());
	Settings.endGroup();

	Settings.beginGroup("Headers");
	Settings.setValue("jobs", Options.value("Jobs").toList());
	Settings.setValue("docs", Options.value("Docs").toList());
	Settings.endGroup();


	delete ui;
}

void MainWindow::updateDocRoles(const QString& Path, bool Addnew)
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

	setEnabled(false);

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

	QtConcurrent::blockingMap(Documents, [&Data, &Roles, &Updates, &Locker] (QPair<int, QString>& Row) -> void
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

				return;
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

	setEnabled(true);
}

void MainWindow::updateJobRoles(const QString& Path, bool Addnew)
{
	QSqlQuery Query("SELECT id, nazwa FROM rodzajeopr", Db);

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

	setEnabled(false);

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

	Query.prepare("INSERT INTO rodzajeopr (nazwa) VALUES (?)");

	if (Addnew) for (const auto& Role : Newroles)
	{
		Query.addBindValue(Role);
		Query.exec();

		Roles.insert(Role, Query.lastInsertId().toInt());
	}

	Query.prepare("SELECT id, numer FROM operaty");

	if (Query.exec()) while (Query.next()) Documents.append(
	{
		Query.value(0).toInt(),
		Query.value(1).toString()
	});

	QtConcurrent::blockingMap(Documents, [&Data, &Roles, &Updates, &Locker] (QPair<int, QString>& Row) -> void
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

				return;
			}
		}
	});

	Query.prepare("UPDATE operaty SET rodzaj = ? WHERE id = ?");

	for (const auto& Row : Updates)
	{
		Query.addBindValue(Row.second);
		Query.addBindValue(Row.first);
		Query.exec();

		QApplication::processEvents();
	}

	setEnabled(true);
}

void MainWindow::wheelEvent(QWheelEvent* Event)
{
	QMainWindow::wheelEvent(Event);

	if (CurrentDoc && QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
	{
		if (Event->angleDelta().y() > 0) zoomInClicked();
		else if (Event->angleDelta().y() < 0) zoomOutClicked();
	}
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

void MainWindow::importClicked(void)
{
	ImportDialog* Dialog = new ImportDialog(this);

	connect(Dialog, &ImportDialog::accepted,
		   Dialog, &ImportDialog::deleteLater);

	connect(Dialog, &ImportDialog::rejected,
		   Dialog, &ImportDialog::deleteLater);

	connect(Dialog, &ImportDialog::onLoadRequest,
		   this, &MainWindow::importData);

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

void MainWindow::exportClicked(void)
{
	const QString Path = QFileDialog::getExistingDirectory(this, tr("Select directory"));

	if (Path.isEmpty()) return;

	QSqlQuery Query(Db);
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
		Queue.push_back(Index);
		Index = Queue.takeFirst();
	}

	Queue.push_back(Index);

	focusDocument(Index.first, Index.second);
}

void MainWindow::prevClicked(void)
{
	auto Index = Queue.takeLast();

	if (CurrentDoc == Index.first && Queue.size())
	{
		Queue.push_front(Index);
		Index = Queue.takeLast();
	}

	Queue.push_front(Index);

	focusDocument(Index.first, Index.second);
}

void MainWindow::saveClicked(void)
{
	QSqlQuery delQuery(Db), updQuery(Db), addQuery(Db), docQuery(Db);
	int Err(0);

	delQuery.prepare("DELETE FROM zmiany WHERE id = ?");

	updQuery.prepare("UPDATE zmiany SET arkusz = ?, obreb = ?, stare = ?, nowe = ?, uwagi = ? WHERE id = ?");

	addQuery.prepare("INSERT INTO zmiany (dokument, arkusz, obreb, stare, nowe, uwagi) VALUES (?, ?, ?, ?, ?, ?)");

	docQuery.prepare("UPDATE dokumenty SET operator = ?, data = CURRENT_TIMESTAMP WHERE id = ?");

	for (const auto& Change : cwidget->getChanges(CurrentDoc)) switch (Change.value("status").toInt())
	{
		case 1:
			addQuery.addBindValue(Change.value("did"));
			addQuery.addBindValue(Change.value("sheet"));
			addQuery.addBindValue(Change.value("area"));
			addQuery.addBindValue(Change.value("before").toStringList().join(';'));
			addQuery.addBindValue(Change.value("after").toStringList().join(';'));
			addQuery.addBindValue(Change.value("comment").toString());

			addQuery.exec();
		break;
		case 2:
			updQuery.addBindValue(Change.value("sheet"));
			updQuery.addBindValue(Change.value("area"));
			updQuery.addBindValue(Change.value("before").toStringList().join(';'));
			updQuery.addBindValue(Change.value("after").toStringList().join(';'));
			updQuery.addBindValue(Change.value("comment").toString());
			updQuery.addBindValue(Change.value("uid"));

			updQuery.exec();
		break;
		case 3:
			delQuery.addBindValue(Change.value("uid"));

			delQuery.exec();
		break;
		case -1: ++Err; break;
	}

	docQuery.addBindValue(AppCommon::getCurrentUser());
	docQuery.addBindValue(CurrentDoc);

	docQuery.exec();

	if (Err) QMessageBox::warning(this, tr("Saving changes"), tr("There are %n unsaved changes(s) due incomplete data.", nullptr, Err));

	emit onSaveChanges(CurrentDoc);
}

void MainWindow::editClicked(void)
{
	QSqlQuery Query(Db);

	Query.prepare("INSERT INTO blokady (id, operator) "
			    "VALUES (?, ?)");

	Query.addBindValue(CurrentDoc);
	Query.addBindValue(AppCommon::getCurrentUser());

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

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());
}

void MainWindow::lockClicked(void)
{
	const QString Types = Options.value("Types").toStringList().join(',');
	const int Count = Options.value("Count", 1).toInt();
	const QString User = AppCommon::getCurrentUser();

	QVector<QString> dNames, jNames;
	QMap<int, QList<int>> IDS;

	QSqlQuery Query("LOCK TABLES blokady WRITE, operaty AS o READ, dokumenty AS d READ, rodzajedok AS r READ", Db);

	Query.prepare(QString("SELECT DISTINCT o.id FROM operaty o "
					  "INNER JOIN dokumenty d ON d.operat = o.id "
					  "INNER JOIN rodzajedok r ON d.rodzaj = r.id "
					  "WHERE d.rodzaj IN (%1) AND d.data IS NULL "
					  "AND d.id NOT IN (SELECT id FROM blokady) "
					  "LIMIT %2")
			    .arg(Types).arg(Count));

	if (Query.exec()) while (Query.next())
	{
		IDS.insert(Query.value(0).toInt(), QList<int>());
	}

	Query.prepare(QString("SELECT d.nazwa, d.id, o.numer, o.id FROM dokumenty d "
					  "INNER JOIN operaty o ON d.operat = o.id "
					  "INNER JOIN rodzajedok r ON d.rodzaj = r.id "
					  "WHERE o.id = ? AND d.rodzaj IN (%1) AND d.data IS NULL "
					  "AND d.id NOT IN (SELECT id FROM blokady)")
			    .arg(Types));

	for (auto i = IDS.begin(); i != IDS.end(); ++i)
	{
		Query.addBindValue(i.key());

		if (Query.exec()) while (Query.next())
		{
			const QString dName = Query.value(0).toString();
			const QString oName = Query.value(2).toString();

			const int dID = Query.value(1).toInt();
			const int oID = Query.value(3).toInt();

			lwidget->appendDocument(dName, dID, oName, oID);
			Queue.append({ dID, oID });
			Locked.insert(dID);

			i.value().append(dID);
		}
	}

	Query.prepare("INSERT INTO blokady (id, operator) VALUES (?, ?)");

	for (const auto& List : IDS) for (const auto& ID : List)
	{
		Query.addBindValue(ID);
		Query.addBindValue(User);

		Query.exec();
	}

	Query.exec("UNLOCK TABLES");

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());
}

void MainWindow::unlockClicked(void)
{
	const int Chg = cwidget->getChanges().size();

	if (Chg) switch (QMessageBox::question(this, tr("Unlocking document"),
								    tr("There are %n unsaved changes(s).", nullptr, Chg),
								    QMessageBox::Save | QMessageBox::Ignore | QMessageBox::Abort))
	{
		case QMessageBox::Save: saveClicked(); break;
		case QMessageBox::Abort: return; default:;
	}

	QSqlQuery Query(Db);

	Query.prepare("DELETE FROM blokady WHERE id = ?");
	Query.addBindValue(CurrentDoc);
	Query.exec();

	for (int i = 0; i < Queue.size(); ++i)
	{
		if (Queue[i].first == CurrentDoc)
		{
			Queue.removeAt(i--);
		}
	}

	Locked.remove(CurrentDoc);
	cwidget->discardChanged(CurrentDoc);

	ui->actionAddchange->setEnabled(false);
	ui->actionRemovechange->setEnabled(false);
	ui->actionUndochange->setEnabled(false);

	ui->actionLock->setEnabled(true);
	ui->actionSave->setEnabled(false);
	ui->actionUnlock->setEnabled(false);

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());

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

	Scale = double(Img.height()) / double(Image.height());

	ui->label->setPixmap(Img);
}

void MainWindow::rotateLeftClicked(void)
{
	if ((Rotation -= 90) <= -360) Rotation = 0;

	ui->label->setPixmap(Image.scaledToHeight(int(Scale* Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::rotateRightClicked(void)
{
	if ((Rotation += 90) >= 360) Rotation = 0;

	ui->label->setPixmap(Image.scaledToHeight(int(Scale* Image.height()))
					 .transformed(QTransform().rotate(Rotation)));
}

void MainWindow::saveRotClicked(void)
{
	auto Img = Image.transformed(QTransform().rotate(Rotation));
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

	ui->actionRotateleft->setEnabled(Index);
	ui->actionRotateright->setEnabled(Index);
	ui->actionFit->setEnabled(Index);
	ui->actionOrg->setEnabled(Index);
	ui->actionZoomin->setEnabled(Index);
	ui->actionZoomout->setEnabled(Index);
	ui->actionSaverotation->setEnabled(Index);

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

	Query.prepare("INSERT INTO dokumenty (nazwa, sciezka, operat) VALUES (?, ?, ?)");

	for (auto i = Docs.constBegin(); i != Docs.constEnd(); ++i)
	{
		const auto ID = Jobs.value(i.key());

		for (const auto& Path : i.value())
		{
			Query.addBindValue(QFileInfo(Path).fileName());
			Query.addBindValue(Path);
			Query.addBindValue(ID);
			Query.exec();
		}

		QApplication::processEvents();
	}
}

void MainWindow::updateRoles(int Objecttype, const QString& Path, bool Addnew)
{
	if (Objecttype == 0) updateDocRoles(Path, Addnew);
	else if (Objecttype == 1) updateJobRoles(Path, Addnew);
}

void MainWindow::importData(const QVariantMap& Jobs, const QVariantMap& Docs)
{
	QHash<QString, int> jRoles;
	QHash<QString, int> dRoles;
	QHash<QString, int> jUids;

	QSqlQuery Query(Db);

	setEnabled(false);

	Query.prepare("SELECT id, nazwa FROM rodzajeopr");

	if (Query.exec()) while (Query.next()) jRoles.insert(Query.value(1).toString(),
											   Query.value(0).toInt());

	Query.prepare("INSERT INTO rodzajeopr (nazwa) VALUES (?)");

	for (const auto& Job : Jobs) if (!jRoles.contains(Job.toString()))
	{
		Query.addBindValue(Job);

		if (Query.exec()) jRoles.insert(Job.toString(),
								  Query.lastInsertId().toInt());

		QApplication::processEvents();
	}

	Query.prepare("SELECT id, nazwa FROM rodzajedok");

	if (Query.exec()) while (Query.next()) dRoles.insert(Query.value(1).toString(),
											   Query.value(0).toInt());

	Query.prepare("INSERT INTO rodzajedok (nazwa) VALUES (?)");

	for (const auto& Doc : Docs) if (!dRoles.contains(Doc.toStringList().value(1)))
	{
		Query.addBindValue(Doc.toStringList().value(1));

		if (Query.exec()) dRoles.insert(Doc.toStringList().value(1),
								  Query.lastInsertId().toInt());

		QApplication::processEvents();
	}

	Query.prepare("SELECT id, numer FROM operaty");

	if (Query.exec()) while (Query.next()) jUids.insert(Query.value(1).toString(),
											  Query.value(0).toInt());

	Query.prepare("INSERT INTO operaty (numer, rodzaj) VALUES (?, ?)");

	for (auto i = Jobs.constBegin(); i != Jobs.constEnd(); ++i) if (!jUids.contains(i.key()))
	{
		Query.addBindValue(i.key());
		Query.addBindValue(jRoles.value(i.value().toString()));

		if (Query.exec()) jUids.insert(i.key(),
								 Query.lastInsertId().toInt());

		QApplication::processEvents();
	}

	Query.prepare("INSERT INTO dokumenty (nazwa, sciezka, rodzaj, operat) VALUES (?, ?, ?, ?)");

	for (auto i = Docs.constBegin(); i != Docs.constEnd(); ++i)
	{
		if (!QFile::exists(i.key())) continue;

		const QStringList Data = i.value().toStringList();

		Query.addBindValue(QFileInfo(i.key()).fileName());
		Query.addBindValue(QFileInfo(i.key()).absoluteFilePath());
		Query.addBindValue(dRoles.value(Data.value(1)));
		Query.addBindValue(jUids.value(Data.value(0)));

		Query.exec();

		QApplication::processEvents();
	}

	setEnabled(true);
}
