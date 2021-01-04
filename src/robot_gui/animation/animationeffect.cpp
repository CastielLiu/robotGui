#include "animationeffect.h"

AnimationEffect::AnimationEffect()
{
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

void AnimationEffect::fadeOut(QPainter *painter, const QRect &rect,
                                 float factor, const QPixmap &pixmap_now,
                                 const QPixmap &pixmap_next)
{
    painter->setOpacity(1);
    painter->drawPixmap(rect, pixmap_next);
    painter->setOpacity(factor);
    painter->drawPixmap(rect, pixmap_now);
}

void AnimationEffect::blinds(QPainter *painter, const QRect &rect,
                 float factor, const QPixmap &pixmap_now,
                 const QPixmap &pixmap_next)
{

    painter->drawPixmap(rect, pixmap_now);
    int n = 5;
    int dh = pixmap_now.height() / n + 0.5;
    int ddh = ((1.0-factor) * dh)+0.5;
    if(ddh < 0 ) return;

    for(int i = 0; i < n; i++)
        painter->drawPixmap(rect.x(), rect.y() + i * dh, pixmap_next,
                            0, i * dh, pixmap_next.width(), ddh);

}

void AnimationEffect::flipLeftToRight(QPainter *painter, const QRect &rect,
                          float factor, const QPixmap &pixmap_now,
                          const QPixmap &pixmap_next)
{
    int w = rect.width();
    int h = rect.height();

    float rot = factor * 90.0f;
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

