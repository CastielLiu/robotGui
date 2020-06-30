#include<mainwindow.h>

void MainWindow::onActionUserId()
{
    QInputDialog dia(this);
    dia.setWindowTitle("Set User Id");
    dia.setLabelText("Please input id: ");
    dia.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput
    dia.setTextValue(QString::number(g_myId));
    if(dia.exec() == QInputDialog::Accepted)
    {
       g_myId = dia.textValue().toUInt();
       if(g_myId == 0)
       {
           ui->statusBar->showMessage("error, please input again",1000);
           onActionUserId(); //输入错误后再次调用本函数
       }
       QString msg = QString("Set User Id ") + QString::number(g_myId) + " ok.";
       ui->statusBar->showMessage(msg, 3000);

       this->updateStatusBarPemanentMsg();
    }
}

void MainWindow::onActionAutoLogin()
{
    int button = QMessageBox::question(this, tr("自动登录设置"),
                                   QString(tr("下次自动登录？")),
                                   QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::Yes)
        m_autoRegister = true;
    else
        m_autoRegister = false;
}

void MainWindow::onActionRobotCallId()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("设置机器人通话ID")); //窗口名称
    dialog.setLabelText("请输入: "); //输入提示
    dialog.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput IntInput
    dialog.setTextValue(QString::number(g_calledId));

    if(dialog.exec() == QInputDialog::Accepted)
    {
        bool ok;
        uint16_t id = dialog.textValue().toUInt(&ok);
        if(ok)
        {
            g_calledId = id;
            this->updateStatusBarPemanentMsg();
        }
        else
            onActionRobotCallId();
    }
}

void MainWindow::onActionRobotControlId()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("设置机器人控制ID")); //窗口名称
    dialog.setLabelText("请输入: "); //输入提示
    dialog.setInputMode(QInputDialog::TextInput);//可选参数：DoubleInput  TextInput IntInput
    dialog.setTextValue(QString::number(g_robotControlId));

    if(dialog.exec() == QInputDialog::Accepted)
    {
        bool ok;
        uint16_t id = dialog.textValue().toUInt(&ok);
        if(ok)
        {
            g_robotControlId = id;
            this->updateStatusBarPemanentMsg();
        }
        else
            onActionRobotControlId();
    }
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this,tr("关于"),tr("castiel_liu@outlook.com"));
}

void MainWindow::onActionWorkLog()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_WorkLogPage);
}

void MainWindow::onActionDebugConfig()
{
    ui->checkBox_canCalled->setCheckState(Qt::CheckState::Checked);
    ui->checkBox_ignoreCalledOffline->setCheckState(Qt::CheckState::Unchecked);

    ui->stackedWidget->setCurrentIndex(stackWidget_DebugPage);
}

//工具：生物雷达
void MainWindow::onActionBiologicalRadar()
{
    ui->stackedWidget->setCurrentIndex(stackWidget_BioRadarPage);
    ui->label_bioRadarTitle->setAlignment(Qt::AlignCenter);

    if(g_isRemoteTerminal) //作为远程端
    {
        ui->widget_bioRadarSerial->hide(); //隐藏串口widget
    }
    else //本地端
    {
        ui->widget_bioRadarSerial->show(); //显示串口widget
        updateAvailaleSerial(); //刷新可用串口
    }
}

//高级
void MainWindow::onActionServerConfig()
{
    bool ok = false;

    while(true)
    {
        QString text = QInputDialog::getText(this, "身份验证","请输入密码",
                                             QLineEdit::Password,
                                             "",&ok);
        if(!ok) break;
        if(text.isEmpty() || text!="huaman")
        {
            QMessageBox::warning(this, tr("警告"),tr("密码错误"),
                                 QMessageBox::Ok,QMessageBox::Ok);
            continue;
        }

        QString tip = g_serverIp.toString() + ":" + QString::number(g_registerPort);
        while(true)
        {
            QString ipPort =
            QInputDialog::getText(this, tr("服务器配置"),
                                  tr("请输入服务器地址和端口号\n例如192.168.0.100:8080"),
                                  QLineEdit::Normal,
                                  tip,&ok);
            if(!ok) break;
            QStringList qlist = ipPort.split(":");
            if(qlist.size()==2)
            {
                QHostAddress tempAdress;
                bool converOk = tempAdress.setAddress(qlist[0]);
                if(converOk)
                {
                    quint16 port = qlist[1].toShort(&converOk);
                    if(converOk)
                    {
                        g_serverIp = tempAdress;
                        g_registerPort = port;
                        QMessageBox::information(this, tr("设置成功"),tr("设置成功"),
                                                 QMessageBox::Ok,QMessageBox::Ok);
                        break;
                    }
                }
            }
            QMessageBox::warning(this, tr("警告"),tr("输入格式错误！"),
                                 QMessageBox::Ok,QMessageBox::Ok);
        }
        break;
    }
}

