/***************************************************************************************
 *
 * Qt5 Propeller 2 text browser dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#include "textbrowserdlg.h"
#include "ui_textbrowserdlg.h"

TextBrowserDlg::TextBrowserDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextBrowserDlg)
{
    ui->setupUi(this);
}

TextBrowserDlg::~TextBrowserDlg()
{
    delete ui;
}

void TextBrowserDlg::set_text(const QString& text)
{
    ui->textBrowser->setPlainText(text);
}
