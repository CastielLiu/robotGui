#ifndef ANIMATIONEFFECT_H
#define ANIMATIONEFFECT_H
#include <QPainter>
#include <QRect>

/*



 */

class AnimationEffect
{
/* @brief 动画效果函数指针
 * @param QPainter 绘画器
 * @param QRect 绘画范围
 * @param float factor 比例因子1.0-0.0
 * @param QPixmap now_pixmap 当前画布
 * @param QPixmap next_pixmap 下一张
 */
typedef void (*effectFun_t)(QPainter *, const QRect &,float,
                            const QPixmap &,const QPixmap &);
public:
    AnimationEffect();

    enum EffectType
    {
        type_FadeOut,
        type_Blinds,
        type_flipL2R,
        type_moveL2R,
        type_moveR2L,
        type_out2In,
        type_in2out,
    };

    size_t effectCnt;
    EffectType type;
    std::vector<effectFun_t> effectList;
    effectFun_t effect;

    void setEffectType(EffectType _type);

    //淡出
    static void fadeOut(QPainter *painter, const QRect &rect,
                     float factor, const QPixmap &pixmap_now,
                     const QPixmap &pixmap_next);

    static void blinds(QPainter *painter, const QRect &rect,
                     float factor, const QPixmap &pixmap_now,
                     const QPixmap &pixmap_next);

    //从左到右翻转
    static void flipLeftToRight(QPainter *painter, const QRect &rect,
                              float factor, const QPixmap &pixmap_now,
                              const QPixmap &pixmap_next);

    static void outsideToInside(QPainter *painter, const QRect &rect,
                                  float factor, const QPixmap &pixmap_now,
                                  const QPixmap &pixmap_next);

    static void insideToOutside(QPainter *painter, const QRect &rect,
                              float factor, const QPixmap &pixmap_now,
                              const QPixmap &pixmap_next);

    static void moveLeftToRight(QPainter *painter, const QRect &rect,
                                float factor, const QPixmap &pixmap_now,
                                const QPixmap &pixmap_next);

    static void moveRightToLeft(QPainter *painter, const QRect &rect,
                                float factor, const QPixmap &pixmap_now,
                                const QPixmap &pixmap_next);
};


#endif // ANIMATIONEFFECT_H
