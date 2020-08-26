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

    // 设置图片列表;
    void setImageList(QStringList imageFileNameList);
    // 添加图片;
    void addImage(QString imageFileName);

    // 设置按钮是否可见;
    void setControlButtonVisible(bool isVisible);
    // 设置按钮背景色;
    void setControlButtonColor(QColor buttonColor);

    // 设置是否显示边框;
    void setBorderVisible(bool isVisible);
    // 设置边框颜色;
    void setBorderColor(QColor borderColor);
    // 设置边框宽度;
    void setBorderWidth(int borderWidth);
    // 设置边框弧度;
    void setBorderRadian(int borderRadian);

    // 设置图片切换时长;
    void setImageChangeDuration(int duration);

    // 开始播放;
    // 全部属性设置结束之后再调用startPlay();
    void startPlay();
    void disableImageChange(bool flag){m_isPauseChange = flag;}

signals:
    // 图片点击信号;
    void clicked();

private:
    void initAnimation();// 初始化
    void initButtonAnimation();

    void initChangeImageButton();// 初始化图片切换按钮;

    void paintEvent(QPaintEvent *event);// 绘图事件;

    void mousePressEvent(QMouseEvent* event);// 鼠标点击事件;
public slots:
    void onImageChangeTimeout(bool manual=false); // 图片切换时钟;

    // 图片切换按钮点击;
    void onImageSwitchButtonClicked(int buttonId);

private:
    // 用来做图片切换滑动效果，目前以透明度作为切换效果;
    QScrollArea * m_imagePlayWidget;
    // 图片列表;
    QList<QString> m_imageFileNameList;

    QTimer m_imageChangeTimer; // 图片切换时钟;
    int m_imageChangeDuration; // 图片切换周期;
    int m_animationDuration;   // 切换过程周期

    // 当前显示图片index;
    int m_currentDrawImageIndx;

    // 切换图片;
    QPixmap m_currentPixmap;
    QPixmap m_nextPixmap;

    // 图片切换动画类;
    QPropertyAnimation* m_animationObj;
    AnimationEffect m_animationEffect;

    QPropertyAnimation* m_buttonAnimation;
    QPushButton* m_moveButton;

    // 按钮列表;
    QList<QPushButton*> m_pButtonChangeImageList;
    QWidget* m_buttonBackWidget;
    QColor m_buttonBackColor;

    // 边框属性;
    QColor m_borderColor;
    bool m_isShowBorder;
    int m_borderWidth;
    int m_borderRadian;

    bool m_isPauseChange;
};

#endif
