#include "audiohandler.h"

AudioHandler::AudioHandler():
    m_input(nullptr),
    m_tempRecorderBuf(nullptr)
{
     std::cout << "create AudioHandler in thread: " << QThread::currentThreadId() << std::endl;
}

AudioHandler::~AudioHandler()
{
    if(m_tempRecorderBuf!=nullptr)
        delete [] m_tempRecorderBuf;
    if(m_input != nullptr)
    {
        m_input->stop();
        delete m_input;
    }
}

bool AudioHandler::init(const std::string& mode)
{
    if(mode == "record")//录制
        startRead(8000,1,16);
    else if(mode == "play")//播放
        configPlayer(8000,1,16,100);
    else
    {
        qDebug() << "AudioHandler init mode: play or record!";
        return false;
    }
    return true;
}

/* 以下为捕获声音相关函数 */
void AudioHandler::startRead(int samplerate, int channelcount, int samplesize)
{
    //std::cout << "start reading audio..." << std::endl;
    m_tempRecorderBuf = new char[960];

    QAudioFormat format = setAudioFormat(samplerate,channelcount,samplesize);
    m_input = new QAudioInput(format,this);
    m_inputDevice = m_input->start();

    connect(m_inputDevice,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
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

void AudioHandler::onReadyRead()
{
    // read audio from input device
    qint64 len = m_inputDevice->read(m_tempRecorderBuf,960);
    if(len <=0 )
        return;
    QMutexLocker locker(&m_audioSendMutex);

    //录制缓冲区溢出，清空数据并重置发送指针
    if(m_recoderBuffer.size() > MAX_AUDIO_BUF_LEN )
    {
        m_recoderBuffer.clear();
        m_currentSendIndex = 0;
        return;
    }
    m_recoderBuffer.append(m_tempRecorderBuf,len);
}

//发送60ms的音频数据
/*
 * sockect   @sockect指针
 * receiverId @消息接受者id
 */
void AudioHandler::sendAudio(QUdpSocket* sockect, uint16_t receiverId)
{
    QMutexLocker locker(&m_audioSendMutex);

    //剩余语音长度小于60ms，不发送
    if(m_recoderBuffer.size() < m_currentSendIndex + AUDIO_LEN_60ms)
        return;
    else
    {
        //从m_recoderBuffer中拷贝60ms的音频并发送
        char *sendData = new char[sizeof(transPack_t)+AUDIO_LEN_60ms];

        transPack_t *package = (transPack_t *)sendData;
        package->head[0] = 0x66;
        package->head[1] = 0xcc;
        package->type = Audio;
        package->length = AUDIO_LEN_60ms;
        package->checkNum = 0;
        package->senderId = g_myId;
        package->receiverId = receiverId;

        memcpy(sendData+sizeof(transPack_t),&m_recoderBuffer.data()[m_currentSendIndex], AUDIO_LEN_60ms);
        sockect->writeDatagram(sendData,sizeof(transPack_t)+AUDIO_LEN_60ms,g_serverIp,g_msgPort);

        //std::cout << "send data in thread: " << QThread::currentThreadId()  << "g_msgPort:" << g_msgPort << std::endl;
        m_currentSendIndex += AUDIO_LEN_60ms;
        //qDebug()<<m_currentSendIndex;
        delete []sendData;

        //发送索引超出最大期望值，重置缓冲区
        if(m_currentSendIndex > MAX_AUDIO_BUF_LEN)
        {
            // 将m_recoderBuffer右侧剩余的字节赋值给新的m_recoderBuffer
            m_recoderBuffer = m_recoderBuffer.right(m_recoderBuffer.size()-MAX_AUDIO_BUF_LEN);
            m_currentSendIndex -= MAX_AUDIO_BUF_LEN;
        }
    }
}


/* 以下为播放声音相关函数 */

void AudioHandler::configPlayer(int sampleRate, int channelCount, int sampleSize,int volumn)
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

    m_OutPut = new QAudioOutput(nFormat);
    m_OutPut->setVolume(volumn);
    m_AudioIo = m_OutPut->start();
}

//将收到的语音消息添加进语音缓冲区，等待播放
void AudioHandler::appendData(char* const buf, int len)
{
    QMutexLocker locker(&m_audioPlayMutex);
    m_PCMDataBuffer.append(buf, len);
    //qDebug()<< "m_PCMDataBuffer.append";
}

void AudioHandler::playAudio()
{
    QMutexLocker locker(&m_audioPlayMutex);
    //qDebug()<<m_currentPlayIndex << "\t" << m_PCMDataBuffer.size();
    //剩余语音长度小于60ms，不播放
    if(m_PCMDataBuffer.size() < m_currentPlayIndex + AUDIO_LEN_60ms)
        return;
    else
    {
        //从m_PCMDataBuffer中拷贝60ms的音频并播放
       /* char *writeData = new char[AUDIO_LEN_60ms];
        memcpy(writeData,&m_PCMDataBuffer.data()[m_currentPlayIndex], AUDIO_LEN_60ms);
        m_AudioIo->write(writeData, AUDIO_LEN_60ms);
        delete [] writeData;*/

        m_AudioIo->write(&m_PCMDataBuffer.data()[m_currentPlayIndex], AUDIO_LEN_60ms);
        m_currentPlayIndex += AUDIO_LEN_60ms;

        //索引超出最大期望值，重置缓冲区
        if(m_currentPlayIndex > MAX_AUDIO_BUF_LEN)
        {
            // 将m_PCMDataBuffer右侧剩余的字节赋值给新的m_PCMDataBuffer
            m_PCMDataBuffer = m_PCMDataBuffer.right(m_PCMDataBuffer.size()-MAX_AUDIO_BUF_LEN);
            m_currentPlayIndex -= MAX_AUDIO_BUF_LEN;
        }
    }
}

