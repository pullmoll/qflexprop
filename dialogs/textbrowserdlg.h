/***************************************************************************************
 *
 * Qt5 Propeller 2 text browser dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QDialog>

namespace Ui {
class TextBrowserDlg;
}

class TextBrowserDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TextBrowserDlg(QWidget *parent = nullptr);
    ~TextBrowserDlg();

    void set_text(const QString& text);
private:
    Ui::TextBrowserDlg *ui;
};
