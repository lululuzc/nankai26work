#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include <QWidget>
#include <QRect>

class MenuScreen : public QWidget
{
    Q_OBJECT

public:
    explicit MenuScreen(QWidget *parent = nullptr);

signals:
    void singlePlayerRequested();
    void twoPlayerRequested();
    void aiModeRequested();
    void quitRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QRect m_singleBtn;
    QRect m_twoBtn;
    QRect m_aiBtn;
    bool m_hoverSingle = false;
    bool m_hoverTwo = false;
    bool m_hoverAi = false;
};

#endif // MENUSCREEN_H
