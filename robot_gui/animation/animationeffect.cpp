#include "animationeffect.h"

AnimationEffect::AnimationEffect()
{
    //将所有效果添加到效果列表
    //应与EffectType保持一致
    effectList.push_back(fadeOut);
    effectList.push_back(blinds);
    effectList.push_back(flipLeftToRight);
    effectList.push_back(outsideToInside);
    effectList.push_back(insideToOutside);
    effectList.push_back(moveLeftToRight);
    effectList.push_back(moveRightToLeft);
    effectCnt = effectList.size();
    type = EffectType(0); //default
    effect = effectList[type];
}

void AnimationEffect::setEffectType(EffectType _type)
{
    type = _type;
    effect = effectList[type];
}

//淡出
void AnimationEffect::fadeOut(QPainter *painter, const QRect &rect,
                                 float factor, const QPixmap &pixmap_now,
                                 const QPixmap &pixmap_next)
{
    painter->setOpacity(1);
    painter->drawPixmap(rect, pixmap_next);
    painter->setOpacity(factor); //不透明度
    painter->drawPixmap(rect, pixmap_now);
}

//百叶窗
void AnimationEffect::blinds(QPainter *painter, const QRect &rect,
                 float factor, const QPixmap &pixmap_now,
                 const QPixmap &pixmap_next)
{
    //首先绘制下一张图片
    painter->drawPixmap(rect, pixmap_now);
    int n = 5; //百叶窗栅格数
    int dh = pixmap_now.height() / n + 0.5; //纵向均分得到每个栅格的高度
    int ddh = ((1.0-factor) * dh)+0.5; //由比例因子确定覆盖高度
    if(ddh < 0 ) return; //ddh为零将导致绘图参数错误而全部绘制！
    // 遍历百叶窗进行绘制
    for(int i = 0; i < n; i++)
        painter->drawPixmap(rect.x(), rect.y() + i * dh, pixmap_next,
                            0, i * dh, pixmap_next.width(), ddh);

}

//从左到右翻转
void AnimationEffect::flipLeftToRight(QPainter *painter, const QRect &rect,
                          float factor, const QPixmap &pixmap_now,
                          const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    float rot = factor * 90.0f; //旋转角度
    QTransform trans;
    trans.translate(w * (1 - factor), h / 2);
    trans.rotate(rot, Qt::YAxis);
    trans.translate(-w, -h / 2);

    painter->setTransform(trans);
    painter->drawPixmap(rect.x(), rect.y(), pixmap_next);
    painter->resetTransform();

    trans.reset();
    rot = 90 * (factor - 1);
    trans.translate(w * (1 - factor), h / 2);
    trans.rotate(rot, Qt::YAxis);
    trans.translate(0, -h / 2);

    painter->setTransform(trans);
    painter->drawPixmap(rect.x(), rect.y(), pixmap_now);
    painter->resetTransform();
}

void AnimationEffect::outsideToInside(QPainter *painter, const QRect &rect,
                              float factor, const QPixmap &pixmap_now,
                              const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    painter->drawPixmap(rect, pixmap_next);

    int dy = h * (1.0-factor) / 2 +0.5;
    int dh = h - 2 * dy + 0.5;
    if(dh == 0) return;
    painter->drawPixmap(rect.x(),rect.y()+dy, pixmap_now, 0, dy, pixmap_now.width(), dh);
}

void AnimationEffect::insideToOutside(QPainter *painter, const QRect &rect,
                          float factor, const QPixmap &pixmap_now,
                          const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    painter->drawPixmap(rect, pixmap_now);

    int dy = h * factor / 2 +0.5;
    int dh = h - 2 * dy + 0.5;
    if(dh == 0) return;
    painter->drawPixmap(rect.x(),rect.y()+dy, pixmap_next, 0, dy, pixmap_next.width(), dh);
}

void AnimationEffect::moveLeftToRight(QPainter *painter, const QRect &rect,
                            float factor, const QPixmap &pixmap_now,
                            const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    int left_w = w * factor + 0.5;
    int right_w = w - left_w;

    if(right_w > 0)
       painter->drawPixmap(rect.x()+left_w,rect.y(), pixmap_next.copy
                            (0,0,right_w,pixmap_next.height()));
    if(left_w > 0)
        painter->drawPixmap(rect.x(),rect.y(), pixmap_now.copy
                            (right_w,0,left_w,pixmap_now.height()));
}

void AnimationEffect::moveRightToLeft(QPainter *painter, const QRect &rect,
                            float factor, const QPixmap &pixmap_now,
                            const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    int left_w = w * (1.0-factor) + 0.5;
    int right_w = w - left_w;

    if(right_w > 0)
       painter->drawPixmap(rect.x()+left_w,rect.y(), pixmap_now.copy
                            (0,0,right_w,pixmap_now.height()));
    if(left_w > 0)
        painter->drawPixmap(rect.x(),rect.y(), pixmap_next.copy
                            (right_w,0,left_w,pixmap_next.height()));
}

