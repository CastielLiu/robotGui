#include "audiohandler.h"

AudioHandler::AudioHandler():
    m_sampleRate(8000),
    m_channelCount(1),
    m_sampleSize(16),
    m_audioTimePerFrame(40),
    m_audioSizePerFrame(m_sampleRate*m_channelCount*m_sampleSize/8*m_audioTimePerFrame/1000),
    m_maxAudioBufLen(m_audioSizePerFrame*200),
    m_isAudioOpen(false),
    m_input(nullptr)
{
     //std::cout << "create AudioHandler in thread: " << QThread::currentThreadId() << std::endl;
     m_audioBuffer = new char[m_maxAudioBufLen];
}

AudioHandler::~AudioHandler()
{
    stop(m_audioMode);

    if(m_file.isOpen())
        m_file.close();

    delete [] m_audioBuffer;
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
    m_input = new QAudioInput(device,format,this);

    m_inputDevice = m_input->start();
    if(!m_inputDevice->isOpen())
    {
        qDebug() << "open input audio device failed!";
        m_input->stop();
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

    QMutexLocker locker(&m_audioSendMutex);

    int remained = m_maxAudioBufLen - m_writeIndex; //buf末尾空闲位置个数

    qint64 len;
    if(remained < m_audioSizePerFrame) //剩余空间不足以放下一帧语音
        len =m_inputDevice->read(m_audioBuffer+m_writeIndex,remained); //只读出一部分
    else
        len =m_inputDevice->read(m_audioBuffer+m_writeIndex,m_audioSizePerFrame);

    //qDebug() << "read audio len :" << len ;
    m_writeIndex += len; //更新写指针位置
    m_writeIndex%=m_maxAudioBufLen;
}

/*
 * sockect   @sockect指针
 * receiverId @消息接受者id
 */
void AudioHandler::sendAudio(QUdpSocket* sockect, uint16_t receiverId)
{
    if(!m_isAudioOpen) return;

    QMutexLocker locker(&m_audioSendMutex);

    //剩余语音长度不足一帧，不发送
    int canSendLen = (m_writeIndex-m_readIndex+m_maxAudioBufLen)%m_maxAudioBufLen;
    if(canSendLen < m_audioSizePerFrame)
        return;

    //从buffer中拷贝一帧音频并发送
    char *sendData = new char[sizeof(pkgHeader_t)+m_audioSizePerFrame];

    pkgHeader_t *package = (pkgHeader_t *)sendData;
    package->head[0] = 0x66;
    package->head[1] = 0xcc;
    package->type = PkgType_Audio;
    package->length = m_audioSizePerFrame;
    package->checkNum = 0;
    package->senderId = g_myId;
    package->receiverId = receiverId;

    memcpy(sendData+sizeof(pkgHeader_t),m_audioBuffer+m_readIndex , m_audioSizePerFrame);
    m_readIndex += m_audioSizePerFrame;
    m_readIndex %= m_maxAudioBufLen;
    locker.unlock();

    sockect->writeDatagram(sendData,sizeof(pkgHeader_t)+m_audioSizePerFrame,g_serverIp,g_msgPort);

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

    if (m_OutPut != nullptr) delete m_OutPut;

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
    m_OutPut = new QAudioOutput(device, nFormat);

    m_OutPut->setVolume(volumn);
    m_outputDevice = m_OutPut->start();

    if(!m_outputDevice->isOpen())
    {
        qDebug() << "open output audio device failed!";
        m_OutPut->stop();
        return false;
    }

    return true;
}

//将收到的语音消息添加进语音缓冲区，等待播放
void AudioHandler::appendData(char* const buf, int len)
{
/*
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
*/

    QMutexLocker locker(&m_audioPlayMutex);

    int remained = m_maxAudioBufLen - m_writeIndex; //buf末尾空闲位置个数
    if(remained < len) //空间不足
    {
        memcpy(m_audioBuffer+m_writeIndex,buf,remained); //先拷贝一部分到buf尾部
        memcpy(m_audioBuffer,buf+remained,len-remained); //再拷贝剩下的到buf头部
    }
    else //空间足够，直接拷贝
        memcpy(m_audioBuffer+m_writeIndex,buf,len);
    m_writeIndex += len; //更新写指针位置
    m_writeIndex = m_writeIndex%m_maxAudioBufLen;
    //qDebug() << "appendData len: " <<len << "\t"
    //         << "m_writeIndex: " << m_writeIndex;
}

void AudioHandler::playAudio()
{
    if(!m_isAudioOpen) return;

    QMutexLocker locker(&m_audioPlayMutex);
    int canPlayLen = (m_writeIndex-m_readIndex+m_maxAudioBufLen)%m_maxAudioBufLen;
    if(canPlayLen < 1*m_audioSizePerFrame) //可播放长度小于一帧音频长度的n倍时，暂不播放
        return;

    qint64 len = m_outputDevice->write(m_audioBuffer+m_readIndex , canPlayLen);
/*
    qDebug() << "play audio :" << QDateTime::currentMSecsSinceEpoch() << "\t"
             << "can play len: " << canPlayLen << "\t"
             << "play len:" << len <<"\t"
             << "writeIndex: " << m_writeIndex << "\t"
             << "readIndex:" << m_readIndex << "\t"
             << "m_maxAudioBufLen:" << m_maxAudioBufLen;

*/
    m_readIndex += len;
    m_readIndex = m_readIndex%m_maxAudioBufLen;
}

