#include "carouselimagewindow.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

CarouselImageWindow::CarouselImageWindow(QWidget *parent)
    : QWidget(parent)
    , m_currentDrawImageIndx(0)
    , m_animationObj(nullptr)
    , m_buttonBackColor(Qt::white)
    , m_borderColor(Qt::red)
    , m_isShowBorder(true)
    , m_borderWidth(5)
    , m_borderRadian(3)
    , m_isPauseChange(false)
{
    m_buttonBackWidget = new QWidget;
    m_buttonBackWidget->setStyleSheet(".QWidget{background:transparent;}");

    if(parent != nullptr)
        this->setGeometry(parent->rect());
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    //qDebug() << this->geometry();

    m_imageChangeDuration = 3000;
    m_animationDuration = 700;

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
    m_buttonAnimation->setLoopCount(-1);
    m_buttonAnimation->start();
}

void CarouselImageWindow::initAnimation()
{
    if(m_animationObj != nullptr)
        delete m_animationObj;

    this->setProperty("ImageChangeFactor", 1.0);
    m_animationObj = new QPropertyAnimation(this, "ImageChangeFactor");

    m_animationObj->setDuration(m_animationDuration);

    //m_animationObj->setEasingCurve(QEasingCurve::InOutQuart);

    m_animationObj->setStartValue(1.0);
    m_animationObj->setEndValue(0.0);

    //m_animationObj->setLoopCount(1);
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

    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect imageRect = this->rect();
    //qDebug() << "imageRect: " <<  imageRect;

    if (m_isShowBorder)
        imageRect.adjust(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth);
    //qDebug() << imageRect;

    painter.save();
    if (m_imageFileNameList.isEmpty())
    {
        QPixmap backPixmap = QPixmap(":/Resources/CarouselImageBack.png");
        painter.drawPixmap(imageRect, backPixmap.scaled(imageRect.size()));
    }

    else if (m_imageFileNameList.count() == 1)
    {
        QPixmap backPixmap = QPixmap(m_imageFileNameList.first());
        painter.drawPixmap(imageRect, backPixmap.scaled(imageRect.size()));
    }

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

    painter.restore();
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

void CarouselImageWindow::mousePressEvent(QMouseEvent* event)
{
    //qDebug() << m_currentDrawImageIndx;
    return QWidget::mousePressEvent(event);
}

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
