#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QDebug>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioInput>
#include <QMutex>
#include <QMutexLocker>
#include <QByteArray>
#include <iostream>
#include <string>
#include <QUdpSocket>
#include <globalvariable.h>
#include <QDir>


class AudioHandler : public QObject
{
    Q_OBJECT
public:
    AudioHandler();
    ~AudioHandler();

public:
    enum AudioMode
    {
        AudioMode_Play,
        AudioMode_Record,
    };

    bool init(AudioMode mode );
    bool stop(AudioMode mode);

    void appendData(char* const buf, int len);
    void playAudio();
    void sendAudio(QUdpSocket *sockect, uint16_t receiverId);

private:
    QAudioFormat setAudioFormat(int samplerate, int channelcount, int samplesize);
    bool configReader(int samplerate, int channelcount, int samplesize);
    bool configPlayer(int sampleRate, int channelCount, int sampleSize, qreal volumn);

private slots:
    void onReadyRead();

private:
    const int m_sampleRate;
    const int m_channelCount;
    const int m_sampleSize;
    const uint16_t m_audioTimePerFrame;
    const uint16_t m_audioSizePerFrame;
    const uint32_t m_maxAudioBufLen;

    bool m_isAudioOpen;
    AudioMode m_audioMode;

    char *m_audioBuffer; //存放语音(本地获取的语音/接收到的语音)
    uint16_t m_writeIndex=0;  //写索引位置
    uint16_t m_readIndex=0;   //读索引位置

    //output  播放
    QAudioOutput *m_OutPut = nullptr;
    QIODevice *m_AudioIo = nullptr;
    QMutex m_audioPlayMutex;

    //input  录制
    QAudioInput *m_input;
    QIODevice *m_inputDevice;
    QMutex m_audioSendMutex;

    // 测试
    QFile m_file;
};

#endif // AUDIOHANDLER_H
