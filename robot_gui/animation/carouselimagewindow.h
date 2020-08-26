#ifndef CAROUSE_IMAGE_WINDOW
#define CAROUSE_IMAGE_WINDOW

#include <QWidget>
#include <QScrollArea>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QMouseEvent>
#include <ctime>
#include <cstdlib>
#include "animationeffect.h"

class CarouselImageWindow : public QWidget
{
    Q_OBJECT

public:
    CarouselImageWindow(QWidget *parent = nullptr);
    ~CarouselImageWindow();

    // ����ͼƬ�б�;
    void setImageList(QStringList imageFileNameList);
    // ���ͼƬ;
    void addImage(QString imageFileName);

    // ���ð�ť�Ƿ�ɼ�;
    void setControlButtonVisible(bool isVisible);
    // ���ð�ť����ɫ;
    void setControlButtonColor(QColor buttonColor);

    // �����Ƿ���ʾ�߿�;
    void setBorderVisible(bool isVisible);
    // ���ñ߿���ɫ;
    void setBorderColor(QColor borderColor);
    // ���ñ߿���;
    void setBorderWidth(int borderWidth);
    // ���ñ߿򻡶�;
    void setBorderRadian(int borderRadian);

    // ����ͼƬ�л�ʱ��;
    void setImageChangeDuration(int duration);

    // ��ʼ����;
    // ȫ���������ý���֮���ٵ���startPlay();
    void startPlay();
    void disableImageChange(bool flag){m_isPauseChange = flag;}

signals:
    // ͼƬ����ź�;
    void clicked();

private:
    void initAnimation();// ��ʼ��
    void initButtonAnimation();

    void initChangeImageButton();// ��ʼ��ͼƬ�л���ť;

    void paintEvent(QPaintEvent *event);// ��ͼ�¼�;

    void mousePressEvent(QMouseEvent* event);// ������¼�;
public slots:
    void onImageChangeTimeout(bool manual=false); // ͼƬ�л�ʱ��;

    // ͼƬ�л���ť���;
    void onImageSwitchButtonClicked(int buttonId);

private:
    // ������ͼƬ�л�����Ч����Ŀǰ��͸������Ϊ�л�Ч��;
    QScrollArea * m_imagePlayWidget;
    // ͼƬ�б�;
    QList<QString> m_imageFileNameList;

    QTimer m_imageChangeTimer; // ͼƬ�л�ʱ��;
    int m_imageChangeDuration; // ͼƬ�л�����;
    int m_animationDuration;   // �л���������

    // ��ǰ��ʾͼƬindex;
    int m_currentDrawImageIndx;

    // �л�ͼƬ;
    QPixmap m_currentPixmap;
    QPixmap m_nextPixmap;

    // ͼƬ�л�������;
    QPropertyAnimation* m_animationObj;
    AnimationEffect m_animationEffect;

    QPropertyAnimation* m_buttonAnimation;
    QPushButton* m_moveButton;

    // ��ť�б�;
    QList<QPushButton*> m_pButtonChangeImageList;
    QWidget* m_buttonBackWidget;
    QColor m_buttonBackColor;

    // �߿�����;
    QColor m_borderColor;
    bool m_isShowBorder;
    int m_borderWidth;
    int m_borderRadian;

    bool m_isPauseChange;
};

#endif
