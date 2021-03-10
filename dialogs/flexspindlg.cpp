/***************************************************************************************
 *
 * Qt5 Propeller 2 flexpsin executable settings dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QListWidget>
#include "idstrings.h"
#include "flexspindlg.h"
#include "ui_flexspindlg.h"

FlexspinDlg::FlexspinDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlexspinDlg)
{
    ui->setupUi(this);
    QSettings s;
    s.beginGroup(objectName());
    restoreGeometry(s.value(id_window_geometry).toByteArray());
    s.endGroup();

    setup_connections();
}

FlexspinDlg::~FlexspinDlg()
{
    if (QDialog::Accepted == result()) {
	QSettings s;
	s.beginGroup(objectName());
	s.setValue(id_window_geometry, saveGeometry());
	s.endGroup();
    }
    delete ui;
}

/**
 * @brief Return the settings from the dialog
 * @return FlexspinDlg::Settings structure
 */
FlexspinDlg::Settings FlexspinDlg::settings() const
{
    Settings s;
    bool ok;
    s.executable = ui->le_flexspin->text();
    s.quiet = ui->cb_quiet->isChecked();
    s.optimize = ui->le_optimize->text().toInt(&ok);
    if (!ok)
	s.optimize = 0;
    s.include_paths.clear();
    for (int idx = 0; idx < ui->lw_include_paths->count(); idx++) {
	const QListWidgetItem* it = ui->lw_include_paths->item(idx);
	s.include_paths += it->text();
    }
    s.listing = ui->cb_listing->isChecked();
    s.warnings = ui->cb_warnings->isChecked();
    s.errors = ui->cb_errors->isChecked();
    s.hub_address = ui->le_hubaddress->text().toUInt(&ok, 16);
    if (!ok)
	s.hub_address = 0;
    s.skip_coginit = ui->cb_skip_coginit->isChecked();
    return s;
}

/**
 * @brief Set the settings for the dialog
 * @param s const reference to a FlexspinDlg::Settings structure
 */
void FlexspinDlg::set_settings(const FlexspinDlg::Settings& s)
{
    ui->le_flexspin->setText(s.executable);
    ui->le_optimize->setText(QString::number(s.optimize));
    ui->cb_quiet->setChecked(s.quiet);
    ui->lw_include_paths->clear();
    foreach(const QString& path, s.include_paths)
	ui->lw_include_paths->addItem(path);
    ui->cb_listing->setChecked(s.listing);
    ui->cb_warnings->setChecked(s.warnings);
    ui->cb_errors->setChecked(s.errors);
    ui->le_hubaddress->setText(QString("%1").arg(s.hub_address, 4, 16, QChar('0')));
    ui->cb_skip_coginit->setChecked(s.skip_coginit);
    ui->cb_skip_coginit->setEnabled(s.hub_address > 0);
    setup_dialog();
}

/**
 * @brief Slot called when the text in the flexspin line editor changed
 * @param executable path and name of the executable
 */
void FlexspinDlg::le_flexspin_changed(const QString& executable)
{
    QFileInfo info(executable);
    if (info.isExecutable()) {
	ui->le_flexspin->setStyleSheet(QString());
	setup_dialog();
    } else if (info.exists()) {
        ui->le_flexspin->setStyleSheet(QLatin1String("color: darkgreen;"));
    } else {
	ui->le_flexspin->setStyleSheet(QLatin1String("color: red;"));
    }
}

void FlexspinDlg::add_include_path_triggered(bool checked)
{
    Q_UNUSED(checked)
    QFileDialog dlg(this);
    QSettings s;
    s.beginGroup(id_grp_flexspin);
    QString srcdflt = QString("%1/p2tools").arg(QDir::homePath());
    QString srcdir = s.value(id_sourcedir, srcdflt).toString();
    QString pathname = s.value(id_pathname).toString();
    QStringList history = s.value(id_history).toStringList();
    s.endGroup();

    dlg.setWindowTitle(tr("Select directory to add"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setDirectory(srcdir);
    dlg.setFileMode(QFileDialog::DirectoryOnly);
    dlg.setHistory(history);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setViewMode(QFileDialog::List);
    dlg.selectFile(pathname);

    if (QFileDialog::Accepted != dlg.exec())
        return;

    QStringList paths = dlg.selectedFiles();
    if (paths.isEmpty())
        return;

    pathname = paths.first();
    QFileInfo info(pathname);
    if (!info.exists())
        return;
    if (!info.isDir())
        return;
    if (!ui->lw_include_paths->findItems(info.absoluteFilePath(), Qt::MatchCaseSensitive).isEmpty())
        return;

    s.beginGroup(id_grp_flexspin);
    s.setValue(id_sourcedir, info.absoluteDir().path());
    s.setValue(id_pathname, info.fileName());
    history.insert(0, info.absoluteFilePath());
    s.setValue(id_history, history);
    s.endGroup();
    ui->lw_include_paths->addItem(info.absoluteFilePath());
}

void FlexspinDlg::del_include_path_triggered(bool checked)
{
    Q_UNUSED(checked)
    const int idx = ui->lw_include_paths->currentRow();
    if (idx < 0)
        return;
    QListWidgetItem* it = ui->lw_include_paths->takeItem(idx);
    delete it;
}

/**
 * @brief Slot called when the text in the hub address line editor changed
 * @param address const reference to a QString with the new hub address
 */
void FlexspinDlg::le_hubaddress_changed(const QString& address)
{
    bool ok;
    quint32 addr = address.toUInt(&ok, 16);
    if (!ok || address.isEmpty()) {
	ui->le_hubaddress->setStyleSheet(QLatin1String("color: red;"));
	addr = 0;
    } else if (addr > 0x3ff) {
	ui->le_hubaddress->setStyleSheet(QLatin1String("color: lightred;"));
    } else {
	ui->le_hubaddress->setStyleSheet(QString());
    }
    ui->cb_skip_coginit->setEnabled(addr > 0);
}

/**
 * @brief Connect Ui elements to private slots
 */
void FlexspinDlg::setup_connections()
{
    bool ok;
    ok = connect(ui->le_flexspin, &QLineEdit::textChanged,
            this, &FlexspinDlg::le_flexspin_changed);
    Q_ASSERT(ok);
    ok = connect(ui->le_hubaddress, &QLineEdit::textChanged,
            this, &FlexspinDlg::le_hubaddress_changed);
    Q_ASSERT(ok);

    ok = connect(ui->tb_add, &QToolButton::clicked,
            this, &FlexspinDlg::add_include_path_triggered);
    Q_ASSERT(ok);
    ok = connect(ui->tb_del, &QToolButton::clicked,
            this, &FlexspinDlg::del_include_path_triggered);
    Q_ASSERT(ok);
}

/**
 * @brief Setup the dialog
 * If the binary is defined and executable,
 * set its version information in the version info line editor.
 */
void FlexspinDlg::setup_dialog()
{
    static const QStringList args = {{"--version"}};
    ui->le_version_info->setToolTip(QString());
    QString flexspin = ui->le_flexspin->text();
    QFileInfo info(flexspin);
    if (info.isExecutable()) {
	QProcess process(this);
	process.setProgram(flexspin);
	process.setArguments(args);
	process.start();
	if (process.waitForStarted()) {
	    if (process.waitForFinished()) {
		QStringList lines;
		while (!process.atEnd()) {
		    lines += process.readLine().trimmed();
		}
		if (lines.isEmpty())
		    return;
		ui->le_version_info->setText(lines.last());
		ui->le_version_info->setToolTip(lines.join(QChar::LineFeed));
		return;
	    }
	    ui->le_version_info->setText(tr("Executable '%1' did not finish").arg(flexspin));
	    return;
	}
	ui->le_version_info->setText(tr("Executable '%1' was not started").arg(flexspin));
	return;
    }
    ui->le_version_info->setText(tr("Executable '%1' not found").arg(flexspin));
}
