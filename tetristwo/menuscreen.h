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
    void quitRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QRect m_singleBtn;
    QRect m_twoBtn;
    bool m_hoverSingle = false;
    bool m_hoverTwo = false;
};

#endif // MENUSCREEN_H
