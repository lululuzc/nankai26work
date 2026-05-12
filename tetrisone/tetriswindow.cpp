#include "tetriswindow.h"
#include "tetrisboard.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>
#include <algorithm>

TetrisWindow::TetrisWindow(QWidget *parent)
    : QWidget(parent)
{
    m_colors[0] = QColor(0, 240, 240);
    m_colors[1] = QColor(240, 240, 0);
    m_colors[2] = QColor(160, 0, 240);
    m_colors[3] = QColor(0, 240, 0);
    m_colors[4] = QColor(240, 0, 0);
    m_colors[5] = QColor(0, 0, 240);
    m_colors[6] = QColor(240, 160, 0);

    for (int i = 0; i < 7; ++i) {
        m_colorsLight[i] = m_colors[i].lighter(150);
        m_colorsDark[i] = m_colors[i].darker(150);
    }

    m_board = new TetrisBoard(this);
    m_timer = new QTimer(this);

    connect(m_board, &TetrisBoard::boardChanged, this, &TetrisWindow::onBoardChanged);
    connect(m_board, &TetrisBoard::gameOver, this, &TetrisWindow::onGameOver);
    connect(m_board, &TetrisBoard::scoreChanged, this, &TetrisWindow::onScoreChanged);
    connect(m_timer, &QTimer::timeout, m_board, &TetrisBoard::tick);

    int w = SIDEBAR_X + 160 + BOARD_OFFSET_X;
    int h = BOARD_OFFSET_Y * 2 + CELL_SIZE * TetrisBoard::HEIGHT;
    setFixedSize(w, h);
    setWindowTitle("Tetris");

    setFocusPolicy(Qt::StrongFocus);
    m_board->newGame();
    m_timer->start(500);
}

void TetrisWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), QColor(30, 30, 30));

    drawBoard(painter);
    drawSidebar(painter);
}

void TetrisWindow::drawBoard(QPainter &painter)
{
    painter.fillRect(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2,
                     CELL_SIZE * TetrisBoard::WIDTH + 4,
                     CELL_SIZE * TetrisBoard::HEIGHT + 4,
                     QColor(80, 80, 80));

    for (int y = 0; y < TetrisBoard::HEIGHT; ++y) {
        for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
            QColor bg = QColor(20, 20, 20);
            painter.fillRect(BOARD_OFFSET_X + x * CELL_SIZE,
                             BOARD_OFFSET_Y + y * CELL_SIZE,
                             CELL_SIZE, CELL_SIZE, bg);

            auto type = m_board->cellAt(x, y);
            if (type != TetrisBoard::None) {
                int ci = m_board->pieceColorIdx(type);
                drawBlock(painter, x, y, ci, CELL_SIZE, BOARD_OFFSET_X, BOARD_OFFSET_Y);
            }
        }
    }
}

void TetrisWindow::drawBlock(QPainter &painter, int x, int y, int colorIdx,
                              int cellSize, int offsetX, int offsetY)
{
    int px = offsetX + x * cellSize;
    int py = offsetY + y * cellSize;

    painter.fillRect(px, py, cellSize, cellSize, m_colors[colorIdx]);

    int bevel = cellSize / 8;
    if (bevel < 1) bevel = 1;

    painter.fillRect(px, py, cellSize, bevel, m_colorsLight[colorIdx]);
    painter.fillRect(px, py, bevel, cellSize, m_colorsLight[colorIdx]);

    painter.fillRect(px, py + cellSize - bevel, cellSize, bevel, m_colorsDark[colorIdx]);
    painter.fillRect(px + cellSize - bevel, py, bevel, cellSize, m_colorsDark[colorIdx]);

    painter.fillRect(px, py, cellSize, 1, QColor(255, 255, 255, 60));
    painter.fillRect(px, py, 1, cellSize, QColor(255, 255, 255, 60));
}

void TetrisWindow::drawSidebar(QPainter &painter)
{
    int sx = SIDEBAR_X;
    int sy = BOARD_OFFSET_Y;

    painter.setPen(QColor(200, 200, 200));
    QFont font("Consolas", 12, QFont::Bold);
    painter.setFont(font);

    painter.drawText(sx, sy, "NEXT");
    drawNextPiece(painter);

    sy += 130;
    painter.drawText(sx, sy, QString("SCORE: %1").arg(m_board->score()));
    sy += 25;
    painter.drawText(sx, sy, QString("LEVEL: %1").arg(m_board->level()));
    sy += 25;
    painter.drawText(sx, sy, QString("LINES: %1").arg(m_board->lines()));
    sy += 40;

    painter.drawText(sx, sy, "CONTROLS:");
    sy += 20;
    QFont smallFont("Consolas", 9);
    painter.setFont(smallFont);
    painter.drawText(sx, sy, "← →  Move");
    sy += 18;
    painter.drawText(sx, sy, "↑     Rotate");
    sy += 18;
    painter.drawText(sx, sy, "↓     Soft Drop");
    sy += 18;
    painter.drawText(sx, sy, "Space Hard Drop");
    sy += 18;
    painter.drawText(sx, sy, "P     Pause");
    sy += 18;
    painter.drawText(sx, sy, "R     Restart");

    if (m_board->isGameOver()) {
        QFont goFont("Consolas", 18, QFont::Bold);
        painter.setFont(goFont);
        painter.setPen(QColor(255, 50, 50));
        int boardMidX = BOARD_OFFSET_X + CELL_SIZE * TetrisBoard::WIDTH / 2;
        int textW = 120;
        painter.drawText(boardMidX - textW, 250, textW * 2, 40,
                         Qt::AlignHCenter, "GAME OVER");
    }

    if (m_board->isPaused()) {
        QFont pf("Consolas", 16, QFont::Bold);
        painter.setFont(pf);
        painter.setPen(QColor(255, 255, 100));
        int boardMidX = BOARD_OFFSET_X + CELL_SIZE * TetrisBoard::WIDTH / 2;
        painter.drawText(boardMidX - 80, 200, 160, 40,
                         Qt::AlignHCenter, "PAUSED");
    }
}

void TetrisWindow::drawNextPiece(QPainter &painter)
{
    int sx = SIDEBAR_X;
    int sy = BOARD_OFFSET_Y + 25;
    int previewCellSize = 20;

    auto nextType = m_board->nextPieceType();
    if (nextType == TetrisBoard::None) return;

    const auto &layout = m_board->pieceLayout(0, nextType);
    int size = layout.size();

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (layout[y][x]) {
                int ci = m_board->pieceColorIdx(nextType);
                drawBlock(painter, x, y, ci, previewCellSize, sx, sy);
            }
        }
    }
}

void TetrisWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {
        m_board->newGame();
        m_timer->start(500);
        return;
    }

    if (m_board->isGameOver()) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_P) {
        m_board->setPaused(!m_board->isPaused());
        if (m_board->isPaused())
            m_timer->stop();
        else
            m_timer->start(500);
        return;
    }

    if (m_board->isPaused()) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:  m_board->moveLeft(); break;
    case Qt::Key_Right: m_board->moveRight(); break;
    case Qt::Key_Down:  m_board->moveDown(); break;
    case Qt::Key_Up:    m_board->rotate(); break;
    case Qt::Key_Space: m_board->dropDown(); break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void TetrisWindow::onBoardChanged()
{
    update();
}

void TetrisWindow::onGameOver()
{
    m_timer->stop();
    update();
}

void TetrisWindow::onScoreChanged(int, int, int)
{
    int interval = std::max(50, 500 - (m_board->level() - 1) * 50);
    m_timer->setInterval(interval);
    update();
}
