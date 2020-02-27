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
#include <utils.h>

// 8000hz 单通道语音 16位(2字节)存储 8000*1*2*0.06=960
#define AUDIO_LEN_60ms 960 //60ms语音长度
#define MAX_AUDIO_BUF_LEN 9600 //语音缓冲区大小

class AudioHandler : public QObject
{
    Q_OBJECT
public:
    AudioHandler();
    ~AudioHandler();

public:
    bool init(const std::string& mode );
    void configPlayer(int sampleRate, int channelCount, int sampleSize,int volumn);
    void playAudio();
    void appendData(char* const buf, int len);

    void startRead(int samplerate, int channelcount, int samplesize);
    QAudioFormat setAudioFormat(int samplerate, int channelcount, int samplesize);
    void sendAudio(QUdpSocket *sockect, uint16_t receiverId);

private slots:
    void onReadyRead();

private:
    //output  播放
    QAudioOutput *m_OutPut = nullptr;
    QIODevice *m_AudioIo = nullptr;
    QByteArray m_PCMDataBuffer;
    int m_currentPlayIndex = 0;
    QMutex m_audioPlayMutex;

    //input  录制
    QAudioInput *m_input;
    QIODevice *m_inputDevice;

    QByteArray m_recoderBuffer;
    char *m_tempRecorderBuf;
    int m_currentSendIndex =0;
    QMutex m_audioSendMutex;
};

#endif // AUDIOHANDLER_H
