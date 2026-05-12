#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include <QWidget>
#include <QTimer>
#include "tetrisboard.h"

class MenuScreen;
class DualGame;
class VsAiGame;

class TetrisWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TetrisWindow(QWidget *parent = nullptr);

private slots:
    void onBoardChanged();
    void onGameOver();
    void onScoreChanged(int score, int level, int lines);
    void startSinglePlayer();
    void startTwoPlayer();
    void startAiMode();
    void backFromDual();
    void backFromAi();
    void quitGame();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void drawBlock(QPainter &painter, int x, int y, int colorIdx, int cellSize,
                   int offsetX, int offsetY);
    void drawBoard(QPainter &painter);
    void drawNextPiece(QPainter &painter);
    void drawSidebar(QPainter &painter);

    TetrisBoard *m_board;
    QTimer *m_timer;
    MenuScreen *m_menuScreen;
    DualGame *m_dualGame = nullptr;
    VsAiGame *m_vsAiGame = nullptr;

    static constexpr int CELL_SIZE = 30;
    static constexpr int BOARD_OFFSET_X = 30;
    static constexpr int BOARD_OFFSET_Y = 30;
    static constexpr int SIDEBAR_X = BOARD_OFFSET_X + CELL_SIZE * TetrisBoard::WIDTH + 30;

    int m_singleW;
    int m_singleH;

    QColor m_colors[16];
    QColor m_colorsLight[16];
    QColor m_colorsDark[16];
};

#endif // TETRISWINDOW_H
