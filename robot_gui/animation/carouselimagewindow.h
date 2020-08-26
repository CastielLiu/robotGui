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

    void setImageList(QStringList imageFileNameList);

    void addImage(QString imageFileName);

    void setControlButtonVisible(bool isVisible);
    void setControlButtonColor(QColor buttonColor);

    void setBorderVisible(bool isVisible);

    void setBorderColor(QColor borderColor);

    void setBorderWidth(int borderWidth);

    void setBorderRadian(int borderRadian);

    void setImageChangeDuration(int duration);

    void startPlay();
    void disableImageChange(bool flag){m_isPauseChange = flag;}

signals:
    void clicked();

private:
    void initAnimation();
    void initButtonAnimation();

    void initChangeImageButton();

    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent* event);
public slots:
    void onImageChangeTimeout(bool manual=false);

    void onImageSwitchButtonClicked(int buttonId);

private:

    QScrollArea * m_imagePlayWidget;
    QList<QString> m_imageFileNameList;

    QTimer m_imageChangeTimer;
    int m_imageChangeDuration;
    int m_animationDuration;

    int m_currentDrawImageIndx;

    QPixmap m_currentPixmap;
    QPixmap m_nextPixmap;

    QPropertyAnimation* m_animationObj;
    AnimationEffect m_animationEffect;

    QPropertyAnimation* m_buttonAnimation;
    QPushButton* m_moveButton;

    QList<QPushButton*> m_pButtonChangeImageList;
    QWidget* m_buttonBackWidget;
    QColor m_buttonBackColor;

    QColor m_borderColor;
    bool m_isShowBorder;
    int m_borderWidth;
    int m_borderRadian;

    bool m_isPauseChange;
};

#endif
