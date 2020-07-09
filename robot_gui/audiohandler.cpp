#include "audiohandler.h"

AudioHandler::AudioHandler():
    m_sampleRate(8000),
    m_channelCount(1),
    m_sampleSize(16),
    m_audioTimePerFrame(40),
    m_audioSizePerFrame(m_sampleRate*m_channelCount*m_sampleSize/8*m_audioTimePerFrame/1000),
    m_isAudioOpen(false),
    m_input(nullptr)
{
     //std::cout << "create AudioHandler in thread: " << QThread::currentThreadId() << std::endl;
     m_audioBuffer.reserve(20);
}

AudioHandler::~AudioHandler()
{
    stop(m_audioMode);

    if(m_file.isOpen())
        m_file.close();
}

bool AudioHandler::startAudioTest()
{
    bool ok = configReader(m_sampleRate,m_channelCount,m_sampleSize);
    if(ok)
        ok = configPlayer(m_sampleRate,m_channelCount,m_sampleSize,1.0);
    if(ok)
        m_isAudioOpen = true;
    return ok;
}

void AudioHandler::stopAudioTest()
{
    m_isAudioOpen = false;
    QThread::msleep(500);
    if(m_input != nullptr)
    {
        m_input->stop();
        delete m_input;
        m_input = nullptr;
    }
    if(m_OutPut != nullptr)
    {
        m_OutPut->stop();
        delete m_OutPut;
        m_OutPut = nullptr;
    }
}

bool AudioHandler::init(AudioMode mode)
{
    if (m_isAudioOpen) return false;
    m_audioMode = mode;

    if(mode == AudioMode_Record)//录制
        return (m_isAudioOpen = configReader(m_sampleRate,m_channelCount,m_sampleSize));
    else if(mode == AudioMode_Play)//播放
        return (m_isAudioOpen = configPlayer(m_sampleRate,m_channelCount,m_sampleSize,1.0));
    else
    {
        qDebug() << "AudioHandler init mode: play or record!";
        return false;
    }
}

bool AudioHandler::stop(AudioMode mode)
{
    if(!m_isAudioOpen) return false;

    m_isAudioOpen = false;
    QThread::msleep(100); //等待其他线程正常退出后再释放相关内存
    // 也可设立线程运行标志，线程退出后标志复位，此处等待复位后再继续执行

    if(AudioMode_Record == mode)
    {
        qDebug() << "stop audio record..." ;
        if(m_input != nullptr)
        {
            m_input->stop();
            delete m_input;
            m_input = nullptr;
        }
    }
    else if(AudioMode_Play == mode)
    {
        qDebug() << "stop audio play..." ;
        if(m_OutPut != nullptr)
        {
            m_OutPut->stop();
            delete m_OutPut;
            m_OutPut = nullptr;
        }
    }
    return true;
}

/* 以下为捕获声音相关函数 */
bool AudioHandler::configReader(int samplerate, int channelcount, int samplesize)
{
    //std::cout << "start reading audio..." << std::endl;

    QList<QAudioDeviceInfo> deviceInfo = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    if(deviceInfo.size()<=0)
    {
        qDebug() << "AudioHandler: no input device!";
        return false;
    }

    //qDebug() << "default" <<  QAudioDeviceInfo::defaultInputDevice().deviceName();
    qDebug() << "=================video input device ====================";
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultInputDevice();
    foreach(const QAudioDeviceInfo&audio, deviceInfo)
    {
        qDebug() << audio.deviceName();
        if(-1 != audio.deviceName().indexOf(QString("USB_Microphone")))
            device = audio;
    }

    QAudioFormat format = setAudioFormat(samplerate,channelcount,samplesize);
    //m_input = new QAudioInput(format,this); //default device

    if(device == QAudioDeviceInfo::defaultInputDevice())
    {
        qDebug() << "use default input audio device: " << device.deviceName() ;
    }
    else
    {
        qDebug() << "use customize input audio device: " << device.deviceName();
    }
    qDebug() << "=========================================================";

    if (m_input != nullptr) delete m_input;
    m_input = new QAudioInput(device,format,this);

    m_inputDevice = m_input->start();
    if(!m_inputDevice->isOpen())
    {
        qDebug() << "open input audio device failed!";
        m_input->stop();
        delete m_input;
        m_input = nullptr;
        return false;
    }
    connect(m_inputDevice,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    return true;
}

QAudioFormat AudioHandler::setAudioFormat(int samplerate, int channelcount, int samplesize)
{
    QAudioFormat format;
    format.setSampleRate(samplerate);
    format.setChannelCount(channelcount);
    format.setSampleSize(samplesize);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    return format;
}

// read audio from input device
void AudioHandler::onReadyRead()
{
    //qDebug() << "m_inputDevice->bytesAvailable : " << m_inputDevice->bytesAvailable(); //无效
    //qDebug() << QDateTime::currentMSecsSinceEpoch();

    std::shared_ptr<char> audio_ptr = std::shared_ptr<char>(new char[m_audioSizePerFrame]);
    qint64 len = m_inputDevice->read(audio_ptr.get(),m_audioSizePerFrame);

    if(len <= 0) return ;

    audio_t audio(audio_ptr, len);
    m_audioBuffer.write(audio);
}

/*
 * sockect   @sockect指针
 * receiverId @消息接受者id
 */
void AudioHandler::sendAudio(QUdpSocket* sockect, uint16_t receiverId)
{
    if(!m_isAudioOpen) return;

    if(m_audioBuffer.size() == 0) return;

    char *sendData = new char[sizeof(pkgHeader_t)+m_audioSizePerFrame];
    audio_t audio;
    bool ok = m_audioBuffer.read(audio);
    if(!ok)
    {
        delete []sendData;
        return ;
    }
    static uint16_t seq = 0;
    pkgHeader_t *package = (pkgHeader_t *)sendData;
    package->head[0] = 0x66;
    package->head[1] = 0xcc;
    package->type = PkgType_Audio;
    package->length = audio.len;
    package->checkNum = 0;
    package->senderId = g_myId;
    package->receiverId = receiverId;
    package->seq = ++seq;

    memcpy(sendData+sizeof(pkgHeader_t),audio.data.get(), audio.len);

    sockect->writeDatagram(sendData,sizeof(pkgHeader_t)+audio.len,g_serverIp,g_msgPort);

    //std::cout << "send data in thread: " << QThread::currentThreadId()  << "g_msgPort:" << g_msgPort << std::endl;

    delete []sendData;
}


/* 以下为播放声音相关函数 */
bool AudioHandler::configPlayer(int sampleRate, int channelCount, int sampleSize,qreal volumn)
{
    QMutexLocker locker(&m_audioPlayMutex);

    // Format
    QAudioFormat nFormat;
    nFormat.setSampleRate(sampleRate);
    nFormat.setSampleSize(sampleSize);
    nFormat.setChannelCount(channelCount);
    nFormat.setCodec("audio/pcm");
    nFormat.setSampleType(QAudioFormat::SignedInt);
    nFormat.setByteOrder(QAudioFormat::LittleEndian);

    QList<QAudioDeviceInfo> deviceInfo = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    if(deviceInfo.isEmpty())
    {
        qDebug() << "AudioHandler: no output device!";
        return false;
    }

    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    qDebug() << "=================video output device ====================";
    foreach(const QAudioDeviceInfo&audio, deviceInfo)
    {
        qDebug() << audio.deviceName();
        if(-1 != audio.deviceName().indexOf(QString("USB_Microphone")))
           device = audio;
    }
    if(device == QAudioDeviceInfo::defaultInputDevice())
    {
        qDebug() << "use default output audio device: " << device.deviceName() ;
    }
    else
    {
        qDebug() << "use customize output audio device: " << device.deviceName();
    }
    qDebug() << "=========================================================";
    //m_OutPut = new QAudioOutput(nFormat);//default device
    if (m_OutPut != nullptr) delete m_OutPut;
    m_OutPut = new QAudioOutput(device, nFormat);

    m_OutPut->setVolume(volumn);
    m_outputDevice = m_OutPut->start();

    if(!m_outputDevice->isOpen())
    {
        qDebug() << "open output audio device failed!";
        m_OutPut->stop();
        delete m_OutPut;
        m_OutPut = nullptr;
        return false;
    }

    std::thread t(&AudioHandler::playAudioThread,this);
    t.detach();

    return true;
}

void AudioHandler::stopPlayLocalAudio()
{
    if(m_OutPut != nullptr)
    {
        m_OutPut->stop();
        delete m_OutPut;
        m_OutPut = nullptr;
    }
}

bool AudioHandler::playLocalAudio(const std::string& file)
{
    QDir::currentPath();
    QString file_name = QDir::currentPath() +"/audio";
    qDebug() << file_name;
    static QFile audioFile;
    audioFile.setFileName(file_name);
    bool ok = audioFile.open(QIODevice::ReadOnly);
    if(!ok) return false;
    QAudioFormat nFormat;

    nFormat.setSampleRate(m_sampleRate);
    nFormat.setSampleSize(m_sampleSize);
    nFormat.setChannelCount(m_channelCount);
    nFormat.setCodec("audio/pcm");
    nFormat.setSampleType(QAudioFormat::SignedInt);
    nFormat.setByteOrder(QAudioFormat::LittleEndian);

    QList<QAudioDeviceInfo> deviceInfo = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    if(deviceInfo.isEmpty())
    {
        qDebug() << "AudioHandler: no output device!";
        return false;
    }
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();

    if (m_OutPut != nullptr) delete m_OutPut;
    m_OutPut = new QAudioOutput(device, nFormat);
    m_OutPut->setVolume(1.0);
    m_OutPut->start(&audioFile);
}

//将收到的语音消息添加进语音缓冲区，等待播放
void AudioHandler::appendData(char* const buf, int len)
{
    if(len <=0 ) return;

    std::shared_ptr<char> data = std::shared_ptr<char>(new char[len]);
    memcpy(data.get(),buf, len);
    audio_t audio(data, len);
    m_audioBuffer.write(audio);

/*    static int lastTime = QTime::currentTime().msec();
    int now = QTime::currentTime().msec();
    qDebug() << ".." << (now - lastTime + 1000)%1000;
    lastTime = now;
*/
}

void AudioHandler::saveAudio(const char *const buf, size_t len)
{
    QDir::currentPath();
    if(!m_file.isOpen())
    {
        QString file_name = QDir::currentPath() +"/audio";
        qDebug() << file_name;
        m_file.setFileName(file_name);
        bool ok = m_file.open(QIODevice::WriteOnly);
        qDebug() << file_name << " "  << QString::number(ok);
    }
    m_file.write(buf,len);
}

void AudioHandler::playAudioThread()
{
    while(!m_isAudioOpen)
        QThread::msleep(50);

    while(m_isAudioOpen)
    {
        //以只读的方式从缓冲区中读取数据
        //待写入播放器成功后将其弹出
        //否则下次继续播放
        audio_t audio;
        bool ok = m_audioBuffer.onlyRead(audio);
        if(!ok) continue;

        if(! m_outputDevice->isWritable())
            continue;
        qint64 len = m_outputDevice->write(audio.data.get(), audio.len);
        if(len > 0)
            m_audioBuffer.pop_begin();
        QThread::msleep(10);
    }
}

