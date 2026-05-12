#include "dualgame.h"
#include "tetrisboard.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>
#include <QRandomGenerator>
#include <algorithm>

DualGame::DualGame(QWidget *parent,
                   const QColor *colors, const QColor *colorsLight, const QColor *colorsDark)
    : QWidget(parent),
      m_colors(colors), m_colorsLight(colorsLight), m_colorsDark(colorsDark)
{
    setFocusPolicy(Qt::StrongFocus);

    int boardW = CELL_SIZE * TetrisBoard::WIDTH;
    int sidebarW = 120;
    m_board1OffsetX = 30;
    m_sidebar1X = m_board1OffsetX + boardW + 10;
    int gap = 20;
    m_board2OffsetX = m_sidebar1X + sidebarW + gap;
    m_sidebar2X = m_board2OffsetX + boardW + 10;

    m_board1 = new TetrisBoard(this);
    m_board2 = new TetrisBoard(this);
    m_timer1 = new QTimer(this);
    m_timer2 = new QTimer(this);
    m_inputTimer = new QTimer(this);
    m_inputTimer->setInterval(30);
    connect(m_inputTimer, &QTimer::timeout, this, &DualGame::processInput);

    connect(m_timer1, &QTimer::timeout, m_board1, &TetrisBoard::tick);
    connect(m_timer2, &QTimer::timeout, m_board2, &TetrisBoard::tick);

    connect(m_board1, &TetrisBoard::boardChanged, this, [this]() { update(); });
    connect(m_board2, &TetrisBoard::boardChanged, this, [this]() { update(); });

    connect(m_board1, &TetrisBoard::scoreChanged, this, [this](int, int, int) {
        int interval = std::max(50, 500 - (m_board1->level() - 1) * 50);
        m_timer1->setInterval(interval);
        update();
    });
    connect(m_board2, &TetrisBoard::scoreChanged, this, [this](int, int, int) {
        int interval = std::max(50, 500 - (m_board2->level() - 1) * 50);
        m_timer2->setInterval(interval);
        update();
    });

    connect(m_board1, &TetrisBoard::gameOver, this, [this]() {
        if (m_winner != 0) return;
        endGame();
        update();
    });
    connect(m_board2, &TetrisBoard::gameOver, this, [this]() {
        if (m_winner != 0) return;
        endGame();
        update();
    });

    connect(m_board1, &TetrisBoard::linesCleared, this, [this](int count) {
        if (count > 1)
            m_board2->addGarbageLines(count - 1);
    });
    connect(m_board2, &TetrisBoard::linesCleared, this, [this](int count) {
        if (count > 1)
            m_board1->addGarbageLines(count - 1);
    });
}

void DualGame::start()
{
    m_winner = 0;

    m_sharedSeq.clear();
    for (int i = 0; i < 500; ++i) {
        m_sharedSeq.append(static_cast<TetrisBoard::PieceType>(
            QRandomGenerator::global()->bounded(1, TetrisBoard::PIECE_COUNT)));
    }
    m_board1->setPieceSequence(&m_sharedSeq);
    m_board2->setPieceSequence(&m_sharedSeq);

    m_board1->newGame();
    m_board2->newGame();
    m_timer1->start(500);
    m_timer2->start(500);
    m_inputTimer->start();
    setFocus();
}

void DualGame::endGame()
{
    m_timer1->stop();
    m_timer2->stop();
    m_inputTimer->stop();
    m_pressedKeys.clear();

    bool p1Over = m_board1->isGameOver();
    bool p2Over = m_board2->isGameOver();

    if (p1Over && !p2Over)
        m_winner = 2;
    else if (!p1Over && p2Over)
        m_winner = 1;
    else if (p1Over && p2Over) {
        if (m_board1->score() > m_board2->score())
            m_winner = 1;
        else if (m_board2->score() > m_board1->score())
            m_winner = 2;
        else
            m_winner = -1;
    }
}

void DualGame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor(30, 30, 30));

    drawBoardAt(painter, m_board1OffsetX, BOARD_OFFSET_Y, m_board1);
    drawBoardAt(painter, m_board2OffsetX, BOARD_OFFSET_Y, m_board2);
    drawSidebarAt(painter, m_sidebar1X, BOARD_OFFSET_Y, m_board1, "P1");
    drawSidebarAt(painter, m_sidebar2X, BOARD_OFFSET_Y, m_board2, "P2");

    if (m_winner != 0) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));
        QFont winFont("Consolas", 28, QFont::Bold);
        painter.setFont(winFont);
        painter.setPen(QColor(255, 215, 0));
        QString text = (m_winner == -1) ? "DRAW!"
                                       : QString("PLAYER %1 WINS!").arg(m_winner);
        painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, text);

        QFont hintFont("Consolas", 14);
        painter.setFont(hintFont);
        painter.setPen(QColor(200, 200, 200));
        int cy = height() / 2;
        painter.drawText(0, cy + 50, width(), 30, Qt::AlignHCenter, "Press ENTER to return");
        painter.drawText(0, cy + 75, width(), 30, Qt::AlignHCenter, "Press R to restart");
        painter.drawText(0, cy + 100, width(), 30, Qt::AlignHCenter, "Press ESC to menu");
    }
}

void DualGame::drawBoardAt(QPainter &painter, int ox, int oy, TetrisBoard *board)
{
    painter.fillRect(ox - 2, oy - 2,
                     CELL_SIZE * TetrisBoard::WIDTH + 4,
                     CELL_SIZE * TetrisBoard::HEIGHT + 4,
                     QColor(80, 80, 80));

    for (int y = 0; y < TetrisBoard::HEIGHT; ++y) {
        for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
            QColor bg = QColor(20, 20, 20);
            painter.fillRect(ox + x * CELL_SIZE,
                             oy + y * CELL_SIZE,
                             CELL_SIZE, CELL_SIZE, bg);

            auto type = board->cellAt(x, y);
            if (type != TetrisBoard::None) {
                int ci = board->pieceColorIdx(type);
                drawBlock(painter, x, y, ci, CELL_SIZE, ox, oy);
            }
        }
    }
}

void DualGame::drawBlock(QPainter &painter, int x, int y, int colorIdx,
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

void DualGame::drawSidebarAt(QPainter &painter, int sx, int sy, TetrisBoard *board,
                              const QString &label)
{
    painter.setPen(QColor(200, 200, 200));
    QFont font("Consolas", 12, QFont::Bold);
    painter.setFont(font);

    painter.drawText(sx, sy, label);
    drawNextPieceAt(painter, sx, sy + 25, board);

    sy += 130;
    painter.drawText(sx, sy, QString("SCORE: %1").arg(board->score()));
    sy += 22;
    painter.drawText(sx, sy, QString("LEVEL: %1").arg(board->level()));
    sy += 22;
    painter.drawText(sx, sy, QString("LINES: %1").arg(board->lines()));

    if (board->isGameOver()) {
        QFont goFont("Consolas", 14, QFont::Bold);
        painter.setFont(goFont);
        painter.setPen(QColor(255, 50, 50));
        painter.drawText(sx, sy + 50, 120, 30, Qt::AlignLeft, "GAME OVER");
    }

    if (board->isPaused()) {
        QFont pf("Consolas", 12, QFont::Bold);
        painter.setFont(pf);
        painter.setPen(QColor(255, 255, 100));
        painter.drawText(sx, sy + 50, 120, 30, Qt::AlignLeft, "PAUSED");
    }
}

void DualGame::drawNextPieceAt(QPainter &painter, int sx, int sy, TetrisBoard *board)
{
    int previewCellSize = 20;

    auto nextType = board->nextPieceType();
    if (nextType == TetrisBoard::None) return;

    const auto &layout = board->pieceLayout(0, nextType);
    int size = layout.size();

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (layout[y][x]) {
                int ci = board->pieceColorIdx(nextType);
                drawBlock(painter, x, y, ci, previewCellSize, sx, sy);
            }
        }
    }
}

void DualGame::keyPressEvent(QKeyEvent *event)
{
    if (m_winner != 0) {
        if (!event->isAutoRepeat()) {
            if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter
                || event->key() == Qt::Key_Escape) {
                emit backToMenu();
            } else if (event->key() == Qt::Key_R) {
                m_winner = 0;
                start();
            }
        }
        return;
    }

    if (event->isAutoRepeat()) return;

    if (event->key() == Qt::Key_R) {
        m_pressedKeys.clear();
        m_board1->newGame();
        m_board2->newGame();
        m_timer1->start(500);
        m_timer2->start(500);
        m_inputTimer->start();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        m_timer1->stop();
        m_timer2->stop();
        m_inputTimer->stop();
        m_pressedKeys.clear();
        emit backToMenu();
        return;
    }

    if (event->key() == Qt::Key_P) {
        bool paused = !m_board1->isPaused();
        m_board1->setPaused(paused);
        m_board2->setPaused(paused);
        if (paused) {
            m_timer1->stop();
            m_timer2->stop();
            m_inputTimer->stop();
            m_pressedKeys.clear();
        } else {
            m_timer1->start(m_timer1->interval());
            m_timer2->start(m_timer2->interval());
            m_inputTimer->start();
        }
        update();
        return;
    }

    if (m_board1->isPaused()) return;

    switch (event->key()) {
    case Qt::Key_Up:    m_board1->rotate(); break;
    case Qt::Key_Space: m_board1->dropDown(); break;
    case Qt::Key_W:     m_board2->rotate(); break;
    case Qt::Key_Q:     m_board2->dropDown(); break;
    case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Down:
    case Qt::Key_A: case Qt::Key_S: case Qt::Key_D:
        m_pressedKeys.insert(event->key());
        break;
    default: break;
    }
}

void DualGame::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    m_pressedKeys.remove(event->key());
}

void DualGame::processInput()
{
    if (m_winner != 0 || m_board1->isPaused()) return;

    for (int key : m_pressedKeys) {
        switch (key) {
        case Qt::Key_Left:  m_board1->moveLeft(); break;
        case Qt::Key_Right: m_board1->moveRight(); break;
        case Qt::Key_Down:  m_board1->moveDown(); break;
        case Qt::Key_A:     m_board2->moveLeft(); break;
        case Qt::Key_D:     m_board2->moveRight(); break;
        case Qt::Key_S:     m_board2->moveDown(); break;
        default: break;
        }
    }
}

void DualGame::focusOutEvent(QFocusEvent *)
{
    m_pressedKeys.clear();
}

QSize DualGame::windowSize()
{
    int boardW = CELL_SIZE * TetrisBoard::WIDTH;
    int sidebarW = 120;
    int p2SidebarX = (30 + boardW + 10) + sidebarW + 20 + boardW + 10;
    int w = p2SidebarX + sidebarW + 30;
    int h = BOARD_OFFSET_Y * 2 + CELL_SIZE * TetrisBoard::HEIGHT;
    return QSize(w, h);
}
