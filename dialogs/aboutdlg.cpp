#include "aboutdlg.h"
#include "ui_aboutdlg.h"

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 %2")
		   .arg(qApp->applicationDisplayName())
		   .arg(qApp->applicationVersion()));
    ui->lbl_application_name->setText(QString("## %1")
				      .arg(qApp->applicationDisplayName()));
    ui->lbl_application_version->setText(QString("### %1")
				      .arg(qApp->applicationVersion()));
}

AboutDlg::~AboutDlg()
{
    delete ui;
}
