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

QHash<QString, bool> SettingsDlg::elements() const
{
    QHash<QString, bool> current_elements = m_elements;
    foreach(const QString& id, m_elements.keys()) {
	QCheckBox* cb = m_checkboxes.value(id, nullptr);
	if (!cb)
	    continue;
	current_elements.insert(id, cb->isChecked());
    }
    return current_elements;
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

void SettingsDlg::set_elements(QHash<QString, bool> elements)
{
    if (m_elements == elements)
	return;

    m_elements = elements;
    foreach(const QString& id, m_elements.keys()) {
	QCheckBox* cb = m_checkboxes.value(id, nullptr);
	if (!cb)
	    continue;
	cb->setChecked(m_elements.value(id));
	QString tooltip = pinout_leds.value(id);
	if (tooltip.isEmpty())
	    continue;
	QString name = id;
	name = name.remove(QLatin1String("id_")).toUpper();
	QString desc = tooltip;
	desc.remove(tr("Status of the "));
	desc.remove(tr("line."));
	cb->setText(QString("%1 (%2)").arg(name).arg(desc));
	cb->setToolTip(tooltip);
    }
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

    m_checkboxes.insert(id_baud_rate, ui->cb_00);
    m_checkboxes.insert(id_parity_data_stop, ui->cb_01);
    m_checkboxes.insert(id_flow_control, ui->cb_02);
    m_checkboxes.insert(id_pwr, ui->cb_03);
    m_checkboxes.insert(id_ri, ui->cb_04);
    m_checkboxes.insert(id_dcd, ui->cb_05);
    m_checkboxes.insert(id_dtr, ui->cb_06);
    m_checkboxes.insert(id_dsr, ui->cb_07);
    m_checkboxes.insert(id_rts, ui->cb_08);
    m_checkboxes.insert(id_cts, ui->cb_09);
    m_checkboxes.insert(id_txd, ui->cb_10);
    m_checkboxes.insert(id_rxd, ui->cb_11);
    m_checkboxes.insert(id_brk, ui->cb_12);
    m_checkboxes.insert(id_fe, ui->cb_13);
    m_checkboxes.insert(id_pe, ui->cb_14);
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
