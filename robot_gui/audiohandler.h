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
#include <circlebuffer.h>
#include <QDir>
#include <QDateTime>


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
    bool startTet();
    void stopTest();
    void testThread();
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

    bool m_isAudioOpen;
    AudioMode m_audioMode;

    typedef struct Audio
    {
        std::shared_ptr<char> data;
        size_t len;
        Audio(std::shared_ptr<char> _data, size_t n)
        {
            data = _data;
            len = n;
        }
        Audio(){}
    } audio_t;

    //存放语音(本地获取的语音/接收到的语音)
    CircleBuffer<audio_t> m_audioBuffer;

    //output  播放
    QAudioOutput *m_OutPut = nullptr;
    QIODevice *m_outputDevice = nullptr;
    QMutex m_audioPlayMutex;

    //input  录制
    QAudioInput *m_input;
    QIODevice *m_inputDevice;
    QMutex m_audioSendMutex;

    // 测试
    QFile m_file;
    bool test_flag;
};

#endif // AUDIOHANDLER_H
