#include "menuscreen.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>

MenuScreen::MenuScreen(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void MenuScreen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor(30, 30, 30));

    int cx = width() / 2;
    int cy = height() / 2;

    QFont titleFont("Consolas", 36, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 240, 240));
    painter.drawText(cx - 110, cy - 120, 250, 50, Qt::AlignHCenter, "俄罗斯方块");

    int btnW = 180;
    int btnH = 50;
    int gap = 30;
    int btnY = cy - 10;

    m_singleBtn = QRect(cx - btnW - gap / 2, btnY, btnW, btnH);
    m_twoBtn = QRect(cx + gap / 2, btnY, btnW, btnH);
    int btnY2 = btnY + btnH + gap;
    m_aiBtn = QRect(cx - btnW / 2, btnY2, btnW, btnH);

    QFont btnFont("Consolas", 18, QFont::Bold);
    painter.setFont(btnFont);

    QColor singleBg = m_hoverSingle ? QColor(0, 180, 180) : QColor(0, 140, 140);
    painter.fillRect(m_singleBtn, singleBg);
    QColor singleBorder = m_hoverSingle ? QColor(0, 240, 240) : QColor(0, 160, 160);
    painter.setPen(singleBorder);
    painter.drawRect(m_singleBtn);
    painter.setPen(Qt::white);
    painter.drawText(m_singleBtn, Qt::AlignCenter, "单人模式");

    QColor twoBg = m_hoverTwo ? QColor(180, 140, 0) : QColor(140, 100, 0);
    painter.fillRect(m_twoBtn, twoBg);
    QColor twoBorder = m_hoverTwo ? QColor(240, 200, 0) : QColor(160, 120, 0);
    painter.setPen(twoBorder);
    painter.drawRect(m_twoBtn);
    painter.setPen(Qt::white);
    painter.drawText(m_twoBtn, Qt::AlignCenter, "双人模式");

    QColor aiBg = m_hoverAi ? QColor(180, 100, 40) : QColor(140, 70, 20);
    painter.fillRect(m_aiBtn, aiBg);
    QColor aiBorder = m_hoverAi ? QColor(240, 160, 80) : QColor(160, 90, 40);
    painter.setPen(aiBorder);
    painter.drawRect(m_aiBtn);
    painter.setPen(Qt::white);
    painter.drawText(m_aiBtn, Qt::AlignCenter, "人机对战");

    painter.setFont(QFont("Consolas", 11));
    painter.setPen(QColor(140, 140, 140));
    painter.drawText(cx - 150, cy + 140, 300, 20, Qt::AlignHCenter, "按下 Q 或 ESC 以退出");
}

void MenuScreen::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Q || event->key() == Qt::Key_Escape) {
        emit quitRequested();
    }
}

void MenuScreen::mousePressEvent(QMouseEvent *event)
{
    if (m_singleBtn.contains(event->pos())) {
        emit singlePlayerRequested();
    } else if (m_twoBtn.contains(event->pos())) {
        emit twoPlayerRequested();
    } else if (m_aiBtn.contains(event->pos())) {
        emit aiModeRequested();
    }
}

void MenuScreen::mouseMoveEvent(QMouseEvent *event)
{
    bool hoverSingle = m_singleBtn.contains(event->pos());
    bool hoverTwo = m_twoBtn.contains(event->pos());
    bool hoverAi = m_aiBtn.contains(event->pos());
    if (hoverSingle != m_hoverSingle || hoverTwo != m_hoverTwo || hoverAi != m_hoverAi) {
        m_hoverSingle = hoverSingle;
        m_hoverTwo = hoverTwo;
        m_hoverAi = hoverAi;
        update();
    }
}
