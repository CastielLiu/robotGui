#include <mainwindow.h>

void MainWindow::on_pushButton_bioRadarExit_clicked()
{
    emit ui->pushButton_radarOpenSerial->clicked(false);
    ui->stackedWidget->setCurrentIndex(stackWidget_ChatPage);
}

void MainWindow::on_pushButton_roscore_clicked()
{
    utils::systemCmd("bash -c roscore");
}

void MainWindow::on_pushButton_remoteCtrl_clicked()
{
    utils::systemCmd(g_appDir.toStdString() + "/../command/remote_control.sh");
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
            ui->statusBar->showMessage("Please set communicate id!",5000);
        else if(g_robotControlId == 0 && g_isRemoteTerminal)
            ui->statusBar->showMessage("Please set robot control id!",5000);
        else if(g_transferStatus != transferStatus_Idle)
            ui->statusBar->showMessage("Transfer is not idle, please try again later!",5000);
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
    ui->stackedWidget->setCurrentIndex(stackWidget_ChatPage);
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
    ui->stackedWidget->setCurrentIndex(stackWidget_ChatPage);
}

void MainWindow::on_pushButton_clearWorkLog_clicked()
{
    ui->textBrowser_log->clear();
}

void MainWindow::on_pushButton_audioTest_clicked(bool checked)
{
    static AudioHandler* audioHandler = nullptr;
    if(checked)
    {
        qDebug() << "start test audio " <<QThread::currentThreadId();
        if(audioHandler!=nullptr)
            delete audioHandler;
        audioHandler = new AudioHandler;
        if(!audioHandler->startAudioTest())
            ui->pushButton_audioTest->setChecked(false);
    }
    else
    {
        qDebug() << "stop test audio " <<QThread::currentThreadId();
        if(audioHandler!=nullptr)
        {
            audioHandler->stopAudioTest();
            //此处不可删除对象，测试线程可能还在运行
        }
    }
}

void MainWindow::on_pushButton_playLocalAudio_clicked(bool checked)
{
    static AudioHandler *audioHandler = nullptr;
    if(checked)
    {
        audioHandler = new AudioHandler;
        if(!audioHandler->playLocalAudio(""))
        {
            ui->pushButton_playLocalAudio->setChecked(false);
            delete audioHandler;
            audioHandler = nullptr;
            return ;
        }
    }
    else
    {
        audioHandler->stopPlayLocalAudio();
    }
}

void MainWindow::on_pushButton_home_clicked()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_HomePage);
}

void MainWindow::on_pushButton_toChatPage_clicked()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_ChatPage);
}

void MainWindow::on_pushButton_toTransportPage_clicked()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_TransportPage);

    if(mNavigation == nullptr)
    {
        mNavigation = new Navigation();

        connect(mNavigation,SIGNAL(updateGoals(std::vector<goalInfo_t>)),
                this, SLOT(onUpdateNavGoalsInfo(std::vector<goalInfo_t>)));

        QString goalsInfoFile = g_appDir + "/../goals/goals.xml";
        mNavigation->loadGoalPointsInfo(goalsInfoFile);
    }
    //QMessageBox::information(this,"infomation",tr("功能正在开发中，敬请关注！"));
    //return;


}

void MainWindow::on_pushButton_goToNavGoal_clicked()
{
    if(mNavigation == nullptr)
    {
        qDebug() << "mNavigation has not been instantiated";
        return;
    }
    std::string goalName = ui->comboBox_navGoalsInfo->currentText().toStdString();
    goalInfo_t goalInfo = mNavigation->getGoalInfoByName(goalName);
    if(!goalInfo.validity)
    {
        QMessageBox::warning(this,"Invalid Goal",tr("所选目标位置无效，请联系管理员！"));
        return ;
    }

    static Fifo *goalInfoFifo = nullptr;
    if(goalInfoFifo == nullptr)
    {
        goalInfoFifo = new Fifo();

        if(!goalInfoFifo->open(g_appDir.toStdString()+"/../fifo/goalInfoFifo", "w"))
        {
            delete goalInfoFifo;
            goalInfoFifo = nullptr;
            QMessageBox::warning(this,"communication Error",tr("打开FIFO通讯管道失败，请联系管理员！"));
            return ;
        }
    }

    int len = goalInfoFifo->send((void*)&goalInfo.pose, sizeof(goalInfo.pose));
    if(sizeof(goalInfo.pose) != len)
    {
        qDebug() << "write to fifo faild!";
        return;
    }

    goalInfo.print();
}
