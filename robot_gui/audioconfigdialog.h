#ifndef AUDIOCONFIGDIALOG_H
#define AUDIOCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class audioconfigdialog;
}

class audioconfigdialog : public QDialog
{
    Q_OBJECT

public:
    explicit audioconfigdialog(QWidget *parent = nullptr);
    ~audioconfigdialog();

private:
    Ui::audioconfigdialog *ui;
};

#endif // AUDIOCONFIGDIALOG_H
