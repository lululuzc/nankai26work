#include "vsaigame.h"
#include "tetrisboard.h"
#include "aiplayer.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>
#include <QRandomGenerator>
#include <algorithm>

VsAiGame::VsAiGame(QWidget *parent,
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

    m_humanBoard = new TetrisBoard(this);
    m_aiBoard = new TetrisBoard(this);
    m_ai = new AIPlayer(m_aiBoard, this);

    m_humanTimer = new QTimer(this);
    m_aiTimer = new QTimer(this);
    m_inputTimer = new QTimer(this);
    m_inputTimer->setInterval(30);
    connect(m_inputTimer, &QTimer::timeout, this, &VsAiGame::processInput);

    connect(m_humanTimer, &QTimer::timeout, m_humanBoard, &TetrisBoard::tick);
    connect(m_aiTimer, &QTimer::timeout, m_aiBoard, &TetrisBoard::tick);

    connect(m_humanBoard, &TetrisBoard::boardChanged, this, [this]() { update(); });
    connect(m_aiBoard, &TetrisBoard::boardChanged, this, [this]() { update(); });

    connect(m_humanBoard, &TetrisBoard::scoreChanged, this, [this](int, int, int) {
        int interval = std::max(50, 500 - (m_humanBoard->level() - 1) * 50);
        m_humanTimer->setInterval(interval);
        update();
    });
    connect(m_aiBoard, &TetrisBoard::scoreChanged, this, [this](int, int, int) {
        int interval = std::max(50, 500 - (m_aiBoard->level() - 1) * 50);
        m_aiTimer->setInterval(interval);
        update();
    });

    connect(m_humanBoard, &TetrisBoard::gameOver, this, [this]() {
        if (m_gameOver) return;
        endGame();
        update();
    });
    connect(m_aiBoard, &TetrisBoard::gameOver, this, [this]() {
        if (m_gameOver) return;
        endGame();
        update();
    });

    connect(m_humanBoard, &TetrisBoard::linesCleared, this, [this](int count) {
        if (count > 1)
            m_aiBoard->addGarbageLines(count - 1);
    });
    connect(m_aiBoard, &TetrisBoard::linesCleared, this, [this](int count) {
        if (count > 1)
            m_humanBoard->addGarbageLines(count - 1);
    });
}

void VsAiGame::start()
{
    m_gameOver = false;
    m_winner = 0;

    m_sharedSeq.clear();
    for (int i = 0; i < 500; ++i) {
        m_sharedSeq.append(static_cast<TetrisBoard::PieceType>(
            QRandomGenerator::global()->bounded(1, TetrisBoard::PIECE_COUNT)));
    }
    m_humanBoard->setPieceSequence(&m_sharedSeq);
    m_aiBoard->setPieceSequence(&m_sharedSeq);

    m_humanBoard->newGame();
    m_aiBoard->newGame();
    m_humanTimer->start(500);
    m_aiTimer->start(500);
    m_inputTimer->start();
    m_ai->start();
    setFocus();
}

void VsAiGame::endGame()
{
    m_gameOver = true;
    m_humanTimer->stop();
    m_aiTimer->stop();
    m_inputTimer->stop();
    m_ai->stop();
    m_pressedKeys.clear();

    bool humanOver = m_humanBoard->isGameOver();
    bool aiOver = m_aiBoard->isGameOver();

    if (humanOver && !aiOver)
        m_winner = -1;
    else if (!humanOver && aiOver)
        m_winner = 1;
    else if (humanOver && aiOver) {
        if (m_humanBoard->score() > m_aiBoard->score())
            m_winner = 1;
        else if (m_aiBoard->score() > m_humanBoard->score())
            m_winner = -1;
        else
            m_winner = 0;
    }
}

void VsAiGame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor(30, 30, 30));

    drawBoardAt(painter, m_board1OffsetX, BOARD_OFFSET_Y, m_humanBoard);
    drawBoardAt(painter, m_board2OffsetX, BOARD_OFFSET_Y, m_aiBoard);
    drawSidebarAt(painter, m_sidebar1X, BOARD_OFFSET_Y, m_humanBoard, "PLAYER");
    drawSidebarAt(painter, m_sidebar2X, BOARD_OFFSET_Y, m_aiBoard, "AI");

    if (m_gameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));
        QFont winFont("Consolas", 28, QFont::Bold);
        painter.setFont(winFont);
        painter.setPen(QColor(255, 215, 0));
        QString text;
        if (m_winner == 0)
            text = "DRAW!";
        else if (m_winner == 1)
            text = "YOU WIN!";
        else
            text = "AI WINS!";
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

void VsAiGame::drawBoardAt(QPainter &painter, int ox, int oy, TetrisBoard *board)
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

void VsAiGame::drawBlock(QPainter &painter, int x, int y, int colorIdx,
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

void VsAiGame::drawSidebarAt(QPainter &painter, int sx, int sy, TetrisBoard *board,
                              const QString &label)
{
    painter.setPen(QColor(200, 200, 200));
    QFont font("Consolas", 12, QFont::Bold);
    painter.setFont(font);

    painter.drawText(sx, sy, label);
    {
        int previewCellSize = 20;
        auto nextType = board->nextPieceType();
        if (nextType != TetrisBoard::None) {
            const auto &layout = board->pieceLayout(0, nextType);
            int size = layout.size();
            int nsy = sy + 25;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    if (layout[y][x]) {
                        int ci = board->pieceColorIdx(nextType);
                        drawBlock(painter, x, y, ci, previewCellSize, sx, nsy);
                    }
                }
            }
        }
    }

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

void VsAiGame::keyPressEvent(QKeyEvent *event)
{
    if (m_gameOver) {
        if (!event->isAutoRepeat()) {
            if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter
                || event->key() == Qt::Key_Escape) {
                emit backToMenu();
            } else if (event->key() == Qt::Key_R) {
                start();
            }
        }
        return;
    }

    if (event->isAutoRepeat()) return;

    if (event->key() == Qt::Key_R) {
        m_humanTimer->stop();
        m_aiTimer->stop();
        m_inputTimer->stop();
        m_ai->stop();
        m_pressedKeys.clear();
        start();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        m_humanTimer->stop();
        m_aiTimer->stop();
        m_inputTimer->stop();
        m_ai->stop();
        m_pressedKeys.clear();
        emit backToMenu();
        return;
    }

    if (event->key() == Qt::Key_P) {
        bool paused = !m_humanBoard->isPaused();
        m_humanBoard->setPaused(paused);
        m_aiBoard->setPaused(paused);
        if (paused) {
            m_humanTimer->stop();
            m_aiTimer->stop();
            m_inputTimer->stop();
            m_ai->stop();
            m_pressedKeys.clear();
        } else {
            m_humanTimer->start(m_humanTimer->interval());
            m_aiTimer->start(m_aiTimer->interval());
            m_inputTimer->start();
            m_ai->start();
        }
        update();
        return;
    }

    if (m_humanBoard->isPaused()) return;

    if (m_humanBoard->isGameOver()) return;

    switch (event->key()) {
    case Qt::Key_Up:    m_humanBoard->rotate(); break;
    case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Down:
        m_pressedKeys.insert(event->key());
        break;
    default: break;
    }
}

void VsAiGame::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    m_pressedKeys.remove(event->key());
}

void VsAiGame::processInput()
{
    if (m_gameOver || m_humanBoard->isPaused()) return;

    for (int key : m_pressedKeys) {
        switch (key) {
        case Qt::Key_Left:  m_humanBoard->moveLeft(); break;
        case Qt::Key_Right: m_humanBoard->moveRight(); break;
        case Qt::Key_Down:  m_humanBoard->moveDown(); break;
        default: break;
        }
    }
}

void VsAiGame::focusOutEvent(QFocusEvent *)
{
    m_pressedKeys.clear();
}

QSize VsAiGame::windowSize()
{
    int boardW = CELL_SIZE * TetrisBoard::WIDTH;
    int sidebarW = 120;
    int p2SidebarX = (30 + boardW + 10) + sidebarW + 20 + boardW + 10;
    int w = p2SidebarX + sidebarW + 30;
    int h = BOARD_OFFSET_Y * 2 + CELL_SIZE * TetrisBoard::HEIGHT;
    return QSize(w, h);
}
