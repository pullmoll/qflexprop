/*****************************************************************************
 *
 * Qt5 serial terminal emulator
 *
 *  Copyleft 2013-2020 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SerTerm; }
QT_END_NAMESPACE

class SerTerm : public QWidget
{
    Q_OBJECT

public:
    SerTerm(QWidget *parent = nullptr);
    ~SerTerm();

signals:
    void update_pinout(bool redo);
    void term_response(QByteArray response);

public slots:
    void set_device(QIODevice* dev);
    void term_set_size(int width, int height);
    void term_set_width(int width);
    void term_set_height(int height);
    void term_toggle_80_132();
    void putch(uchar ch);
    int write(const QByteArray& data);
    int write(const char *data, size_t len);
    void display_text(const QString& filename);
    void display_maps();
    void set_font_family(const QString& family);
    void set_zoom(int percent);
    void reset();

private slots:
    void term_clear();
    void term_width(bool on);

    void geometry_changed();
    void term_fit_best();
    void zoom_original();
    void zoom_in();
    void zoom_out();
    void save_config();
    void load_config();

    void reset_triggered(bool checked = false);
    void version_triggered(bool checked = false);
    void monitor_triggered(bool checked = false);
    void taqoz_triggered(bool checked = false);
    void sendfile_triggered(bool checked = false);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::SerTerm* ui;
    QIODevice* m_dev;				//!< serial port (or tty)
    QString m_font_family;			//!< Terminal font family
    int m_zoom;					//!< Terminal zoom factor
    QString m_download_path;
    bool m_caps_lock;				//!< Keyboard CAPS lock flag
    bool m_num_lock;				//!< Keyboard NUM lock flag
    bool m_scroll_lock;				//!< Keyboard SCROLL lock flag
    bool m_local_echo;				//!< Local echo if true

    QString load_file(const QString& title);
    void reset_prop();
    void setup_signals();
    void setup_terminal();

};
