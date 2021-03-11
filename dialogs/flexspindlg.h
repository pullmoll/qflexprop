/***************************************************************************************
 *
 * Qt5 Propeller 2 flexpsin executable settings dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QDialog>
#include <QAction>

namespace Ui {
class FlexspinDlg;
}

class FlexspinDlg : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
	QString executable;
        QStringList include_paths;
	bool quiet;
	int optimize;
	bool listing;
	bool warnings;
	bool errors;
	quint32 hub_address;
	bool skip_coginit;
    };

    explicit FlexspinDlg(QWidget *parent = nullptr);
    ~FlexspinDlg();

    Settings settings() const;
    void set_settings(const Settings& s);

private slots:
    void setup_dialog();
    void le_hubaddress_changed(const QString& address);
    void le_flexspin_changed(const QString& executable);
    void include_path_changed(int index = -1);
    void add_include_path_triggered(bool checked);
    void del_include_path_triggered(bool checked);

private:
    Ui::FlexspinDlg *ui;

    void setup_connections();
};
