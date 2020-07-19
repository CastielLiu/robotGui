#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QFrame>

namespace Ui {
class KeyBoard;
}

class KeyBoard : public QFrame
{
    Q_OBJECT

public:
    explicit KeyBoard(QWidget *parent = nullptr);
    ~KeyBoard();
private:
    void update();

private slots:
    void on_pushButton_0_clicked();
    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_ok_clicked();

signals:
    void update_signal(const QString& qstr);

private:
    Ui::KeyBoard *ui;
    QString qstr;
};

#endif // KEYBOARD_H
