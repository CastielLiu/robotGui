#include "carouselimagewindow.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

CarouselImageWindow::CarouselImageWindow(QWidget *parent)
    : QWidget(parent)
    , m_currentDrawImageIndx(0)     //当前绘制的图片索引
    , m_animationObj(nullptr)       //动画实例
    , m_buttonBackColor(Qt::white)  //按钮背景色
    , m_borderColor(Qt::red)        //边框颜色
    , m_isShowBorder(true)          //是否显示边框
    , m_borderWidth(5)              //边框宽度
    , m_borderRadian(3)             //边框圆角半径
    , m_isPauseChange(false)        //是否暂停切换图片(暂停动画)
{
    m_buttonBackWidget = new QWidget;
    m_buttonBackWidget->setStyleSheet(".QWidget{background:transparent;}");

    if(parent != nullptr)
        this->setGeometry(parent->rect());//覆盖父类控件
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    //qDebug() << this->geometry();

    m_imageChangeDuration = 3000; //图片切换周期
    m_animationDuration = 700;   //动画切换周期

    connect(&m_imageChangeTimer, SIGNAL(timeout()), this, SLOT(onImageChangeTimeout()));
    //initButtonAnimation(); //test
}

CarouselImageWindow::~CarouselImageWindow(){}

void CarouselImageWindow::initButtonAnimation()
{
    m_moveButton = new QPushButton(this);

    m_buttonAnimation = new QPropertyAnimation(m_moveButton, "geometry");
    //m_buttonAnimation
    m_buttonAnimation->setDuration(2000);
    m_buttonAnimation->setEasingCurve(QEasingCurve::InOutQuart);
    m_buttonAnimation->setStartValue(m_moveButton->geometry());

    QRect endValue = m_moveButton->geometry().adjusted(100,100,100,100);
    m_buttonAnimation->setEndValue(endValue);
    m_buttonAnimation->setLoopCount(-1); // 设置循环次数，-1表示永久循环
    m_buttonAnimation->start(); //开始动画
}

void CarouselImageWindow::initAnimation()
{
    if(m_animationObj != nullptr)
        delete m_animationObj;

    // 设置ImageChangeFactor属性;
    this->setProperty("ImageChangeFactor", 1.0);
    m_animationObj = new QPropertyAnimation(this, "ImageChangeFactor");

    m_animationObj->setDuration(m_animationDuration);
    //变化曲线，默认为线性变换
    //m_animationObj->setEasingCurve(QEasingCurve::InOutQuart);

    m_animationObj->setStartValue(1.0);
    m_animationObj->setEndValue(0.0);

    //m_animationObj->setLoopCount(1); //默认为1
    connect(m_animationObj, SIGNAL(valueChanged(const QVariant&)), this, SLOT(update()));
}


void CarouselImageWindow::initChangeImageButton()
{
    QButtonGroup* changeButtonGroup = new QButtonGroup;
    QHBoxLayout* hLayout = new QHBoxLayout(m_buttonBackWidget);
    hLayout->addStretch();
    for (int i = 0; i < m_imageFileNameList.count(); i++)
    {
        QPushButton* pButton = new QPushButton;
        pButton->setFixedSize(QSize(18, 18));
        pButton->setCheckable(true);
        pButton->setStyleSheet(QString("QPushButton{background:rgb(%1, %2, %3);border-radius:6px;margin:3px;}\
                                QPushButton:checked{border-radius:9px;margin:0px;}").arg(m_buttonBackColor.red()).arg(m_buttonBackColor.green()).arg(m_buttonBackColor.blue()));

        changeButtonGroup->addButton(pButton, i);
        m_pButtonChangeImageList.append(pButton);
        hLayout->addWidget(pButton);
    }
    hLayout->addStretch();
    hLayout->setSpacing(10);
    hLayout->setMargin(0);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(m_buttonBackWidget);
    mainLayout->setContentsMargins(0, 0, 0, 30);

    connect(changeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onImageSwitchButtonClicked(int)));
}

void CarouselImageWindow::startPlay()
{
    initChangeImageButton();
    if (m_imageFileNameList.count() == 1)
        m_pButtonChangeImageList[m_currentDrawImageIndx]->setChecked(true);
    else if (m_imageFileNameList.count() > 1)
    {
        m_pButtonChangeImageList[m_currentDrawImageIndx]->setChecked(true);
        m_currentPixmap = QPixmap(m_imageFileNameList.at(m_currentDrawImageIndx));
        m_imageChangeTimer.start(m_imageChangeDuration);
        update();
    }
    initAnimation();
}

void CarouselImageWindow::onImageChangeTimeout(bool manual)
{
    if(m_isPauseChange && !manual)
        return ;

    m_currentPixmap = QPixmap(m_imageFileNameList.at(m_currentDrawImageIndx));
    m_currentDrawImageIndx++;
    if (m_currentDrawImageIndx >= m_imageFileNameList.count())
        m_currentDrawImageIndx = 0;

    m_nextPixmap = QPixmap(m_imageFileNameList.at(m_currentDrawImageIndx));

    m_pButtonChangeImageList[m_currentDrawImageIndx]->setChecked(true);

    AnimationEffect::EffectType type =
            AnimationEffect::EffectType(rand()%m_animationEffect.effectCnt);
    m_animationEffect.setEffectType(type);

    initAnimation();
    if(m_animationObj)
        m_animationObj->start();
}

void CarouselImageWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //防走样
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect imageRect = this->rect();
    //qDebug() << "imageRect: " <<  imageRect;

    //如果需要绘制图片边框，需要调整(缩小)图片的绘制范围
    //防止图片显示不完全
    if (m_isShowBorder)
        //调整矩形顶点(内收)
        imageRect.adjust(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth);
    //qDebug() << imageRect;

    painter.save();  //保存绘图器状态
    // 如果列表中没有图片，显示默认背景图
    if (m_imageFileNameList.isEmpty())
    {
        QPixmap backPixmap = QPixmap(":/Resources/CarouselImageBack.png");
        painter.drawPixmap(imageRect, backPixmap.scaled(imageRect.size()));
    }
    // 如果只有一张图片，直接绘制
    else if (m_imageFileNameList.count() == 1)
    {
        QPixmap backPixmap = QPixmap(m_imageFileNameList.first());
        painter.drawPixmap(imageRect, backPixmap.scaled(imageRect.size()));
    }
    // 有多张图片，动态切换
    else if (m_imageFileNameList.count() > 1 && !m_currentPixmap.isNull()
                                             && !m_nextPixmap.isNull())
    {
        QPixmap pixmap_now = m_currentPixmap.scaled(imageRect.size());
        QPixmap pixmap_next = m_nextPixmap.scaled(imageRect.size());

        float imageChangeFactor = this->property("ImageChangeFactor").toFloat();
        //qDebug() << imageChangeFactor ;
        m_animationEffect.effect(&painter, imageRect, imageChangeFactor,
                                 pixmap_now,pixmap_next);
    }
    else
    {
        QPixmap backPixmap = QPixmap(m_imageFileNameList.first());
        painter.drawPixmap(imageRect, backPixmap.scaled(imageRect.size()));
    }

    painter.restore(); //恢复绘画器状态
    if (m_isShowBorder)
    {
        qreal adjustedValue = 1.0 * m_borderWidth / 2;
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        QRectF widgetRect = this->rect();
        QRectF ajustedRect = widgetRect.adjusted(adjustedValue, adjustedValue,
                                                 -adjustedValue, -adjustedValue);
        painter.drawRoundedRect(ajustedRect, m_borderRadian, m_borderRadian);
    }
}

void CarouselImageWindow::onImageSwitchButtonClicked(int buttonId)
{
    m_currentDrawImageIndx = buttonId - 1;
    if (m_currentDrawImageIndx == -1)
        m_currentDrawImageIndx = m_imageFileNameList.count() - 1;

    m_imageChangeTimer.stop();
    onImageChangeTimeout(true);
    m_imageChangeTimer.start(m_imageChangeDuration);
}

// 鼠标移动事件
void CarouselImageWindow::mousePressEvent(QMouseEvent* event)
{
    //qDebug() << m_currentDrawImageIndx;
    return QWidget::mousePressEvent(event);
}

//按钮是否可见
void CarouselImageWindow::setControlButtonVisible(bool isVisible)
{
    m_buttonBackWidget->setVisible(isVisible);
}

void CarouselImageWindow::setControlButtonColor(QColor buttonColor)
{
    m_buttonBackColor = buttonColor;
}

void CarouselImageWindow::setBorderVisible(bool isVisible)
{
    m_isShowBorder = isVisible;
}

void CarouselImageWindow::setBorderColor(QColor borderColor)
{
    m_borderColor = borderColor;
}

void CarouselImageWindow::setBorderWidth(int borderWidth)
{
    m_borderWidth = borderWidth;
}

void CarouselImageWindow::setBorderRadian(int borderRadian)
{
    m_borderRadian = borderRadian;
}

void CarouselImageWindow::setImageChangeDuration(int duration)
{
    m_imageChangeDuration = duration;
}

void CarouselImageWindow::setImageList(QStringList imageFileNameList)
{
    m_imageFileNameList = imageFileNameList;
}

void CarouselImageWindow::addImage(QString imageFileName)
{
    m_imageFileNameList.append(imageFileName);
}
