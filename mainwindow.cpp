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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* Parent)
: QMainWindow(Parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->centralWidget->deleteLater();

	QSettings Settings("Multimap", "Zasiegi");

	Settings.beginGroup("Database");
	Db = QSqlDatabase::addDatabase("QMYSQL");
	Db.setUserName(Settings.value("user", "multimap").toString());
	Db.setPassword(Settings.value("password", "multimap").toString());
	Db.setHostName(Settings.value("server", "localhost").toString());
	Db.setDatabaseName(Settings.value("database", "zasiegi").toString());
	Settings.endGroup();

	setEnabled(Db.open());

	image = new ImageDock(this);

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

	addDockWidget(Qt::TopDockWidgetArea, image);
	addDockWidget(Qt::LeftDockWidgetArea, history);
	addDockWidget(Qt::LeftDockWidgetArea, changes);
	addDockWidget(Qt::LeftDockWidgetArea, locks);
	addDockWidget(Qt::LeftDockWidgetArea, jobs);
	addDockWidget(Qt::LeftDockWidgetArea, docs);

	updateImage(QString());
	documentChanged(0);
	jobChanged(0);

	Settings.beginGroup("Window");
	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);
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

	if (isMaximized()) setGeometry(QApplication::desktop()->availableGeometry(this));

	QSqlQuery Query(Db);

	Query.prepare("SELECT o.numer, o.id "
			    "FROM operaty o "
			    "INNER JOIN blokady b "
			    "ON o.id = b.id "
			    "WHERE b.operator = ?");
	Query.addBindValue(AppCommon::getCurrentUser());

	if (Query.exec()) while (Query.next())
	{
		Locked.insert(Query.value(1).toInt());
		Queue.append(Query.value(1).toInt());

		lwidget->appendDocument(Query.value(0).toString(),
						    Query.value(1).toInt());
	}

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());

	dwidget->setVisibleHeaders(Options.value("Docs").toList());
	jwidget->setVisibleHeaders(Options.value("Jobs").toList());

	connect(jwidget, &JobWidget::onIndexChange,
		   dwidget, &DocWidget::setJobIndex);

	connect(jwidget, &JobWidget::onIndexChange,
		   this, &MainWindow::jobChanged);

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

void MainWindow::aboutClicked(void)
{
	AboutDialog* Dialog = new AboutDialog(this);

	connect(Dialog, &AboutDialog::accepted,
		   Dialog, &AboutDialog::deleteLater);

	connect(Dialog, &AboutDialog::rejected,
		   Dialog, &AboutDialog::deleteLater);

	Dialog->open();
}

void MainWindow::importdataClicked(void)
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

void MainWindow::importClicked(void)
{
	struct DATA { QString Ark, Stare, Nowe; int Obr; };

	const auto Splitter = [] (QMap<int, DATA>& Hash, const QStringList& List, bool Mode) -> void
	{
		QRegExp Exp("(?:(\\d+)\\$)?(?:(\\w+)-)?(\\S+)");

		for (const auto& D : List)
		{
			const QStringList Raw = D.split(':');
			if (Raw.size() != 2) continue;

			const int Group = Raw[0].toInt();

			QStringList Lots;
			QString Sheet;
			int Region(0);

			for (const auto& Str : Raw[1].split(';'))
			{
				if (Exp.indexIn(Str) == -1) continue;
				const QStringList Cap = Exp.capturedTexts();

				if (!Region && !Cap[1].isEmpty())
					Region = Cap[1].toInt();

				if (Sheet.isEmpty() && !Cap[2].isEmpty())
					Sheet = Cap[2];

				Lots.append(Cap[3]);
			}

			if (Group && !Lots.isEmpty())
			{
				if (Mode) Hash[Group].Nowe = Lots.join(';');
				else Hash[Group].Stare = Lots.join(';');

				if (Hash[Group].Ark.isEmpty())
					Hash[Group].Ark = Sheet;

				if (Hash[Group].Obr == 0)
					Hash[Group].Obr = Region;
			}
		}
	};

	const QString Path = QFileDialog::getOpenFileName(this, tr("Select source file"));

	if (Path.isEmpty()) return;
	else setEnabled(false);

	QFile Input(Path);
	QTextStream Stream(&Input);
	Input.open(QFile::Text | QFile::ReadOnly);

	QHash<QString, int> Docs;

	QSqlQuery Query(Db);
	Query.prepare("SELECT numer, id FROM operaty");

	if (Query.exec()) while (Query.next())
	{
		Docs.insert(Query.value(0).toString(),
				  Query.value(1).toInt());
	}

	Query.prepare("INSERT INTO zmiany (dokument, arkusz, obreb, stare, nowe) "
			    "VALUES (?, ?, ?, ?, ?)");

	while (!Stream.atEnd())
	{
		const QStringList Row = Stream.readLine().split('\t');

		if (Row.size() != 3) continue;
		const QString& Jobname = Row[0];

		if (!Docs.contains(Jobname)) continue;
		const int Jobid = Docs[Jobname];

		QMap<int, DATA> Hash;

		Splitter(Hash, Row[1].split('#'), false);
		Splitter(Hash, Row[2].split('#'), true);

		for (const auto& D : Hash)
		{
			Query.addBindValue(Jobid);
			Query.addBindValue(D.Ark);
			Query.addBindValue(D.Obr);
			Query.addBindValue(D.Stare.isNull() ? "" : D.Stare);
			Query.addBindValue(D.Nowe.isNull() ? "" : D.Nowe);

			Query.exec();
		}

		QApplication::processEvents();
	}

	setEnabled(true);
}

void MainWindow::exportClicked(void)
{
	struct DATA { QString Gm, Ob, A, B; int Count = 0; };

	const QString Path = QFileDialog::getSaveFileName(this, tr("Select save file"));

	if (Path.isEmpty()) return;
	else setEnabled(false);

	QHash<QString, DATA> Data;
	QSqlQuery Query(Db);

	Query.prepare("SELECT "
			    "o.numer, g.kod, b.numer, z.arkusz, z.stare, z.nowe "
			    "FROM zmiany z "
			    "INNER JOIN dokumenty d "
			    "ON z.dokument = d.id "
			    "INNER JOIN operaty o "
			    "ON d.operat = o.id "
			    "LEFT JOIN obreby b "
			    "ON z.obreb = b.id "
			    "LEFT JOIN gminy g "
			    "ON b.gmina = g.id");

	if (Query.exec()) while (Query.next())
	{
		QString Op = Query.value(0).toString();
		const int Count = Data[Op].Count + 1;

		if (Data[Op].Gm.isEmpty()) Data[Op].Gm = Query.value(1).toString();
		if (Data[Op].Ob.isEmpty()) Data[Op].Ob = Query.value(2).toString();

		const QString Sh = Query.value(3).toString().replace('-', '0');
		const int Obr = Query.value(2).toInt();

		QStringList
				A = Query.value(4).toString().split(';'),
				B = Query.value(5).toString().split(';');

		A.replaceInStrings("X", "", Qt::CaseInsensitive); A.removeAll("");
		B.replaceInStrings("X", "", Qt::CaseInsensitive); B.removeAll("");

		for (auto& S : A) S.push_front(QString("%1$%2-").arg(Obr).arg(Sh));
		for (auto& S : B) S.push_front(QString("%1$%2-").arg(Obr).arg(Sh));

		if (!A.isEmpty())
		{
			if (!Data[Op].A.isEmpty()) Data[Op].A.append('#');
			Data[Op].A.append(QString("%1:%2").arg(Count).arg(A.join(';')));
		}

		if (!B.isEmpty())
		{
			if (!Data[Op].B.isEmpty()) Data[Op].B.append('#');
			Data[Op].B.append(QString("%1:%2").arg(Count).arg(B.join(';')));
		}

		if (!A.isEmpty() || !B.isEmpty()) ++Data[Op].Count;

		QApplication::processEvents();
	}

	QFile File(Path); QTextStream Stream(&File);
	File.open(QFile::WriteOnly | QFile::Text);

	for (auto i = Data.constBegin(); i != Data.constEnd(); ++i)
	{
		Stream << i.key() << '\t'
			  << i.value().A << '\t'
			  << i.value().B << '\n';

		QApplication::processEvents();
	}

	setEnabled(true);
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

	if (CurrentJob == Index && Queue.size())
	{
		Queue.push_back(Index);
		Index = Queue.takeFirst();
	}

	Queue.push_back(Index);

	focusDocument(Index);
}

void MainWindow::prevClicked(void)
{
	auto Index = Queue.takeLast();

	if (CurrentJob == Index && Queue.size())
	{
		Queue.push_front(Index);
		Index = Queue.takeLast();
	}

	Queue.push_front(Index);

	focusDocument(Index);
}

void MainWindow::saveClicked(void)
{
	QSqlQuery delQuery(Db), updQuery(Db), addQuery(Db), docQuery(Db);
	int Err(0);

	delQuery.prepare("DELETE FROM zmiany WHERE id = ?");

	updQuery.prepare("UPDATE zmiany SET arkusz = ?, obreb = ?, stare = ?, nowe = ?, uwagi = ? WHERE id = ?");

	addQuery.prepare("INSERT INTO zmiany (dokument, arkusz, obreb, stare, nowe, uwagi) VALUES (?, ?, ?, ?, ?, ?)");

	docQuery.prepare("UPDATE dokumenty SET operator = ?, data = CURRENT_TIMESTAMP WHERE id = ?");

	for (const auto& Change : cwidget->getChanges(CurrentJob)) switch (Change.value("status").toInt())
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
	docQuery.addBindValue(CurrentJob);

	docQuery.exec();

	if (Err) QMessageBox::warning(this, tr("Saving changes"), tr("There are %n unsaved changes(s) due incomplete data.", nullptr, Err));

	emit onSaveChanges(CurrentJob);
}

void MainWindow::editClicked(void)
{
	QSqlQuery Query(Db);

	Query.prepare("INSERT INTO blokady (id, operator) "
			    "VALUES (?, ?)");

	Query.addBindValue(CurrentJob);
	Query.addBindValue(AppCommon::getCurrentUser());

	if (Query.exec())
	{
		Query.prepare("SELECT o.numer, o.id "
				    "FROM operaty o "
				    "WHERE o.id = ?");

		Query.addBindValue(CurrentJob);

		if (Query.exec() && Query.next())
		{
			Queue.append(CurrentJob);

			lwidget->appendDocument(Query.value(0).toString(),
							    Query.value(1).toInt());
		}

		Locked.insert(CurrentJob);
		jobChanged(CurrentJob);
	}

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());
}

void MainWindow::lockClicked(void)
{
	const QString Types = Options.value("Types").toStringList().join(',');
	const int Count = Options.value("Count", 1).toInt();
	const QString User = AppCommon::getCurrentUser();

	QList<QPair<int, QString>> List;

	QSqlQuery Query("LOCK TABLES blokady WRITE, operaty AS o READ, dokumenty AS d READ, rodzajedok AS r READ", Db);

	Query.prepare(QString("SELECT DISTINCT o.id, o.numer FROM operaty o "
					  "INNER JOIN dokumenty d ON d.operat = o.id "
					  "INNER JOIN rodzajedok r ON d.rodzaj = r.id "
					  "WHERE d.rodzaj IN (%1) AND d.data IS NULL "
					  "AND d.id NOT IN (SELECT id FROM blokady) "
					  "LIMIT %2")
			    .arg(Types).arg(Count));

	if (Query.exec()) while (Query.next()) List.append({
		Query.value(0).toInt(),
		Query.value(1).toString()
	});

	Query.prepare("INSERT INTO blokady (id, operator) VALUES (?, ?)");

	for (const auto& I : List)
	{
		Query.addBindValue(I.first);
		Query.addBindValue(User);

		if (Query.exec())
		{
			lwidget->appendDocument(I.second, I.first);
			Queue.append(I.first);
			Locked.insert(I.first);
		}
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
		case QMessageBox::Abort: return;
		default:;
	}

	QSqlQuery Query(Db);

	Query.prepare("DELETE FROM blokady WHERE id = ?");
	Query.addBindValue(CurrentJob);
	Query.exec();

	Queue.removeAll(CurrentJob);
	Locked.remove(CurrentJob);

	cwidget->discardChanged(CurrentJob);

	ui->actionAddchange->setEnabled(false);
	ui->actionRemovechange->setEnabled(false);
	ui->actionUndochange->setEnabled(false);

	ui->actionLock->setEnabled(true);
	ui->actionSave->setEnabled(false);
	ui->actionUnlock->setEnabled(false);

	ui->actionNext->setEnabled(Queue.size());
	ui->actionPrevious->setEnabled(Queue.size());

	lwidget->removeDocument(CurrentJob);
}

void MainWindow::zoomInClicked(void)
{
	image->zoomIn();
}

void MainWindow::zoomOutClicked(void)
{
	image->zoomOut();
}

void MainWindow::zoomOrgClicked(void)
{
	image->zoomOrg();
}

void MainWindow::zoomFitClicked(void)
{
	image->zoomFit();
}

void MainWindow::rotateLeftClicked(void)
{
	image->rotateLeft();
}

void MainWindow::rotateRightClicked(void)
{
	image->rotateRight();
}

void MainWindow::openfileClicked(void)
{
	image->openFile();
}

void MainWindow::openfolderClicked(void)
{
	image->openFolder();
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

void MainWindow::jobChanged(int Index)
{
	const bool Lock = Locked.contains(Index);

	ui->actionAddchange->setEnabled(Lock);
	ui->actionRemovechange->setEnabled(Lock);
	ui->actionUndochange->setEnabled(Lock);

	ui->actionLock->setEnabled(!Lock && Index);
	ui->actionSave->setEnabled(Lock);
	ui->actionUnlock->setEnabled(Lock);

	cwidget->setDocIndex(Index, !Lock);

	CurrentJob = Index;
}

void MainWindow::documentChanged(int Index)
{
	ui->actionRotateleft->setEnabled(Index);
	ui->actionRotateright->setEnabled(Index);
	ui->actionFit->setEnabled(Index);
	ui->actionOrg->setEnabled(Index);
	ui->actionZoomin->setEnabled(Index);
	ui->actionZoomout->setEnabled(Index);
}

void MainWindow::focusDocument(int Job)
{
	jwidget->setJobIndex(Job);
}

void MainWindow::updateImage(const QString& Path)
{
	const bool Img = QFile(Path).exists();

	if (!Img) image->clear();
	else image->setImage(Path);

	ui->actionFit->setEnabled(Img);
	ui->actionOrg->setEnabled(Img);
	ui->actionZoomin->setEnabled(Img);
	ui->actionZoomout->setEnabled(Img);
	ui->actionRotateleft->setEnabled(Img);
	ui->actionRotateright->setEnabled(Img);
	ui->actionOpenfile->setEnabled(Img);
	ui->actionOpenfolder->setEnabled(Img);
}

void MainWindow::saveSettings(const QVariantHash& Settings)
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

	setEnabled(false);

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

	setEnabled(true);
}

void MainWindow::updateRoles(int Objecttype, const QString& Path, bool Addnew)
{
	if (Objecttype == 0) updateDocRoles(Path, Addnew);
	else if (Objecttype == 1) updateJobRoles(Path, Addnew);
}

void MainWindow::importData(const QVariantHash& Jobs, const QVariantHash& Docs)
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
		const QStringList Data = i.value().toStringList();

		Query.addBindValue(QFileInfo(i.key()).fileName());
		Query.addBindValue(i.key());
		Query.addBindValue(dRoles.value(Data.value(1)));
		Query.addBindValue(jUids.value(Data.value(0)));

		Query.exec();

		QApplication::processEvents();
	}

	setEnabled(true);
}
