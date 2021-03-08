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

void FlexspinDlg::le_flexspin_changed(const QString& binary)
{
    QFileInfo info(binary);
    if (info.isExecutable()) {
	ui->le_flexspin->setStyleSheet(QString());
	setup_dialog();
    } else if (info.exists()) {
	ui->le_flexspin->setStyleSheet(QLatin1String("color: darkgreen;"));
    } else {
	ui->le_flexspin->setStyleSheet(QLatin1String("color: red;"));
    }
}

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


void FlexspinDlg::setup_connections()
{
    connect(ui->le_flexspin, SIGNAL(textChanged(QString)),
	    this, SLOT(le_flexspin_changed(QString)));
    connect(ui->le_hubaddress, SIGNAL(textChanged(QString)),
	    this, SLOT(le_hubaddress_changed(QString)));
}

/**
 * @brief Setup the dialog.
 * If the binary is defined and executable,
 * return its version information.
 */
void FlexspinDlg::setup_dialog()
{
    static const QStringList args = {{"--version"}};
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
		ui->le_version_info->setToolTip(lines.join(QChar::LineFeed));
		ui->le_version_info->setText(lines.last());
	    } else {
		ui->le_version_info->setText(tr("Executable '%1' did not finish").arg(flexspin));
		ui->le_version_info->setToolTip(QString());
	    }
	} else {
	    ui->le_version_info->setText(tr("Executable '%1' was not started").arg(flexspin));
	}
    } else {
	ui->le_version_info->setText(tr("Executable '%1' not found").arg(flexspin));
	ui->le_version_info->setToolTip(QString());
    }
}
