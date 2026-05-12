#ifndef VSAIGAME_H
#define VSAIGAME_H

#include <QWidget>
#include <QTimer>
#include <QSize>
#include <QSet>
#include "tetrisboard.h"

class AIPlayer;

class VsAiGame : public QWidget
{
    Q_OBJECT

public:
    VsAiGame(QWidget *parent,
             const QColor *colors, const QColor *colorsLight, const QColor *colorsDark);
    void start();
    static QSize windowSize();

signals:
    void backToMenu();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void drawBlock(QPainter &painter, int x, int y, int colorIdx, int cellSize,
                   int offsetX, int offsetY);
    void drawBoardAt(QPainter &painter, int ox, int oy, TetrisBoard *board);
    void drawSidebarAt(QPainter &painter, int sx, int sy, TetrisBoard *board,
                       const QString &label);
    void endGame();
    void processInput();

    TetrisBoard *m_humanBoard;
    TetrisBoard *m_aiBoard;
    AIPlayer *m_ai;
    QTimer *m_humanTimer;
    QTimer *m_aiTimer;
    QTimer *m_inputTimer = nullptr;

    const QColor *m_colors;
    const QColor *m_colorsLight;
    const QColor *m_colorsDark;

    int m_winner = 0;
    bool m_gameOver = false;

    QVector<TetrisBoard::PieceType> m_sharedSeq;
    QSet<int> m_pressedKeys;

    static constexpr int CELL_SIZE = 30;
    static constexpr int BOARD_OFFSET_Y = 30;

    int m_board1OffsetX;
    int m_board2OffsetX;
    int m_sidebar1X;
    int m_sidebar2X;
};

#endif // VSAIGAME_H
