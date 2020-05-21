#include <mainwindow.h>

void MainWindow::on_pushButton_bioRadarExit_clicked()
{
    emit ui->pushButton_radarOpenSerial->clicked(false);
    ui->stackedWidget->setCurrentIndex(stackWidget_MainPage);
}

void MainWindow::on_pushButton_roscore_clicked()
{
    utils::systemCmd("gnome-terminal -- 'bash -c roscore'");
}

void MainWindow::on_pushButton_remoteCtrl_clicked()
{
    utils::systemCmd("./../command/remote_control.sh");
}

void MainWindow::on_pushButton_call_clicked()
{
    if(ui->pushButton_call->isChecked())
    {
        bool ok;
        uint16_t callId = ui->lineEdit_calledId->text().toUShort(&ok);
        if(!ok || callId ==0)
        {
            ui->lineEdit_calledId->setText("error!");
            ui->pushButton_call->setChecked(false);
            return;
        }
        g_calledId = callId;

        if(g_registerStatus != RegisterStatus_Ok )
            ui->statusBar->showMessage("Please login firstly!",5000);
        else if(g_calledId == 0 )
            ui->statusBar->showMessage("Please set communicate id",5000);
        else if(g_robotControlId == 0 && g_isRemoteTerminal)
            ui->statusBar->showMessage("Please set robot control id",5000);
        else if(g_systemStatus == SystemOnThePhone)
            ui->statusBar->showMessage("Call in progress!",5000);
        else if(g_calledId == g_myId)
            ui->statusBar->showMessage(QString("Can not call yourself!"),5000);
        else
        {
            this->startChat(g_calledId);
            return;
        }
        ui->pushButton_call->setChecked(false);
    }
    else
    {
        this->stopChat();
    }
}

void MainWindow::on_pushButton_radarOpenSerial_clicked(bool checked)
{
    QString serialPort = ui->comboBox_availaleSerial->currentText();
    if(serialPort.isEmpty())
    {
        ui->pushButton_radarOpenSerial->setChecked(false);
        return;
    }

    if(checked)
    {
        m_radar = new BiologicalRadar();
        connect(m_radar,SIGNAL(updateData(bioRadarData_t)),this,SLOT(onBioRadarUpdateData(bioRadarData_t)));

        bool ok = m_radar->openSerial(serialPort);
        if(!ok)
        {
            delete m_radar;
            m_radar = nullptr;
            ui->pushButton_radarOpenSerial->setChecked(false);
            QMessageBox::warning(this,"WARNING",tr("打开串口失败！\n串口可能被占用！"),QMessageBox::Ok);
            return;
        }
        ui->pushButton_radarOpenSerial->setText(tr("关闭串口"));
        ui->comboBox_availaleSerial->setDisabled(true);
    }
    else
    {
        ui->pushButton_radarOpenSerial->setText(tr("打开串口"));
        ui->pushButton_radarOpenSerial->setChecked(false);
        ui->comboBox_availaleSerial->setDisabled(false);

        if(m_radar !=nullptr)
        {
            m_radar->closeSerial();
            delete m_radar;
            m_radar = nullptr;
        }
    }
}

void MainWindow::on_pushButton_debugPageOk_clicked()
{
    g_canCalled = ui->checkBox_canCalled->isChecked();
    g_ignoreCalledOffline = ui->checkBox_ignoreCalledOffline->isChecked();
    ui->stackedWidget->setCurrentIndex(stackWidget_MainPage);
}

//按键事件
void MainWindow::on_pushButton_Up_pressed()
{
    //qDebug() << "on_pushButton_Up_pressed";
    static QKeyEvent event( QEvent::KeyPress,Qt::Key_Up, Qt::NoModifier);
    emit ui->widget_control1->keyPressEvent(&event);
}
void MainWindow::on_pushButton_Up_released()
{
    static QKeyEvent event( QEvent::KeyRelease,Qt::Key_Up, Qt::NoModifier);
    emit ui->widget_control1->keyReleaseEvent(&event);
}

void MainWindow::on_pushButton_Down_pressed()
{
    static QKeyEvent event( QEvent::KeyPress,Qt::Key_Down, Qt::NoModifier);
    emit ui->widget_control1->keyPressEvent(&event);
}
void MainWindow::on_pushButton_Down_released()
{
    static QKeyEvent event( QEvent::KeyRelease,Qt::Key_Down, Qt::NoModifier);
    emit ui->widget_control1->keyReleaseEvent(&event);
}

void MainWindow::on_pushButton_Left_pressed()
{
    static QKeyEvent event( QEvent::KeyPress,Qt::Key_Left, Qt::NoModifier);
    emit ui->widget_control1->keyPressEvent(&event);
}
void MainWindow::on_pushButton_Left_released()
{
    static QKeyEvent event( QEvent::KeyRelease,Qt::Key_Left, Qt::NoModifier);
    emit ui->widget_control1->keyReleaseEvent(&event);
}

void MainWindow::on_pushButton_Right_pressed()
{
    static QKeyEvent event( QEvent::KeyPress,Qt::Key_Right, Qt::NoModifier);
    emit ui->widget_control1->keyPressEvent(&event);
}
void MainWindow::on_pushButton_Right_released()
{
    static QKeyEvent event( QEvent::KeyRelease,Qt::Key_Right, Qt::NoModifier);
    emit ui->widget_control1->keyReleaseEvent(&event);
}

void MainWindow::on_pushButton_logpageReturn_clicked()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_MainPage);
}
