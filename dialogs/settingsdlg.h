#pragma once
#include <QDialog>
#include <QCheckBox>

namespace Ui {
class SettingsDlg;
}

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDlg(QWidget *parent = nullptr);
    ~SettingsDlg();

    QFont font() const;
    QHash<QString,bool> elements() const;

    void set_font(const QFont& font);
    void set_font(const QString& family, int size = 12, int weight = QFont::Normal);
    void set_elements(QHash<QString,bool> elements);

private slots:
    void setup_dialog();
    void font_index_changed(int index);
    void size_index_changed(int index);
    void weight_index_changed(int index);

private:
    Ui::SettingsDlg *ui;
    QFont m_font;
    QHash<QString,bool> m_elements;
    QHash<QString,QCheckBox*> m_checkboxes;
};
