#include <QFontComboBox>
#include <QSettings>
#include "settingsdlg.h"
#include "ui_settingsdlg.h"
#include "idstrings.h"

SettingsDlg::SettingsDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDlg)
    , m_font()
{
    ui->setupUi(this);
    setup_dialog();
}

SettingsDlg::~SettingsDlg()
{
    delete ui;
}

QFont SettingsDlg::font() const
{
    return m_font;
}

void SettingsDlg::set_font(const QFont& font)
{
    if (m_font == font)
	return;
    int idx;
    m_font = font;
    idx = ui->fontComboBox->findText(font.family());
    if (idx < 0) {
	ui->fontComboBox->setCurrentFont(font);
	idx = ui->fontComboBox->currentIndex();
    }
    ui->fontComboBox->setCurrentIndex(idx);

    idx = ui->cb_size->findData(font.pointSize());
    if (idx < 0)
	idx = ui->cb_size->findData(8);
    ui->cb_size->setCurrentIndex(idx);

    idx = ui->cb_weight->findData(font.weight());
    if (idx < 0)
	idx = ui->cb_weight->findData(QFont::Normal);
    ui->cb_weight->setCurrentIndex(idx);
    ui->te_example->setFont(m_font);
}

void SettingsDlg::set_font(const QString& family, int size, int weight)
{
    set_font(QFont(family, size, weight));
}

void SettingsDlg::setup_dialog()
{
    ui->cb_size->clear();
    for (int size = 6; size <= 48; /* */) {
	ui->cb_size->addItem(QString("%1 pt").arg(size), size);
	if (size < 16) {
	    size++;
	} else if (size < 24) {
	    size += 2;
	} else if (size < 32) {
	    size += 4;
	} else {
	    size += 8;
	}
    }

    ui->cb_weight->clear();
    ui->cb_weight->addItem(tr("Thin"), QFont::Thin);
    ui->cb_weight->addItem(tr("ExtraLight"), QFont::ExtraLight);
    ui->cb_weight->addItem(tr("Light"), QFont::Light);
    ui->cb_weight->addItem(tr("Normal"), QFont::Normal);
    ui->cb_weight->addItem(tr("Medium"), QFont::Medium);
    ui->cb_weight->addItem(tr("DemiBold"), QFont::DemiBold);
    ui->cb_weight->addItem(tr("Bold"), QFont::Bold);
    ui->cb_weight->addItem(tr("ExtraBold"), QFont::ExtraBold);
    ui->cb_weight->addItem(tr("Black"), QFont::ExtraBold);

    ui->te_example->setText(QString("%1\n%2")
			    .arg(tr("The quick brown fox jumps over the lazy dog."))
			    .arg(tr("0123456789 !#$%() *+-/ .,;: <=> ?@")));
    connect(ui->fontComboBox, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(font_index_changed(int)),
	    Qt::UniqueConnection);
    connect(ui->cb_size, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(size_index_changed(int)),
	    Qt::UniqueConnection);
    connect(ui->cb_weight, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(weight_index_changed(int)),
	    Qt::UniqueConnection);

    QSettings s;
    s.beginGroup(objectName());
    restoreGeometry(s.value(id_window_geometry).toByteArray());
    s.endGroup();
}

void SettingsDlg::font_index_changed(int index)
{
    Q_UNUSED(index)
    const QString family = ui->fontComboBox->currentFont().family();
    m_font.setFamily(family);
    ui->te_example->setFont(m_font);
}

void SettingsDlg::size_index_changed(int index)
{
    Q_UNUSED(index)
    const int size = ui->cb_size->currentData().toInt();
    m_font.setPointSize(size);
    ui->te_example->setFont(m_font);
}

void SettingsDlg::weight_index_changed(int index)
{
    Q_UNUSED(index)
    const int weight = ui->cb_weight->currentData().toInt();
    m_font.setWeight(weight);
    ui->te_example->setFont(m_font);
}
