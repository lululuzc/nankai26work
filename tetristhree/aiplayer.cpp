#include "aiplayer.h"
#include <algorithm>
#include <cmath>

AIPlayer::AIPlayer(TetrisBoard *board, QObject *parent)
    : QObject(parent), m_board(board)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(20);
    connect(m_timer, &QTimer::timeout, this, &AIPlayer::makeMove);
}

void AIPlayer::start()
{
    m_lastPiece = m_board->curPieceType();
    m_state = State::Idle;
    m_timer->start();
}

void AIPlayer::stop()
{
    m_timer->stop();
    m_state = State::Idle;
}

void AIPlayer::makeMove()
{
    if (m_board->isGameOver() || m_board->isPaused())
        return;

    TetrisBoard::PieceType cur = m_board->curPieceType();
    if (cur != m_lastPiece) {
        m_lastPiece = cur;
        if (cur != TetrisBoard::None) {
            Move best = findBestMove();
            if (best.score > -1e8) {
                m_targetRot = best.rotation;
                m_targetX = best.x;
                m_state = State::Rotate;
            }
        }
    }

    switch (m_state) {
    case State::Rotate:
        if (m_board->curRotation() != m_targetRot)
            m_board->rotate();
        else
            m_state = State::Shift;
        break;
    case State::Shift: {
        int dx = m_targetX - m_board->curPos().x();
        if (dx < 0)
            m_board->moveLeft();
        else if (dx > 0)
            m_board->moveRight();
        else
            m_state = State::Idle;
        break;
    }
    case State::Idle:
    default:
        break;
    }
}

void AIPlayer::copyGrid(QVector<QVector<TetrisBoard::PieceType>> &dst)
{
    const auto &src = m_board->grid();
    dst.resize(TetrisBoard::HEIGHT);
    for (int y = 0; y < TetrisBoard::HEIGHT; ++y) {
        dst[y] = src[y];
    }
}

AIPlayer::Move AIPlayer::findBestMove()
{
    Move best;
    TetrisBoard::PieceType type = m_board->curPieceType();
    if (type == TetrisBoard::None)
        return best;

    QVector<QVector<TetrisBoard::PieceType>> grid;
    copyGrid(grid);

    for (int rot = 0; rot < 4; ++rot) {
        const auto &layout = m_board->pieceLayout(rot, type);
        int rows = layout.size();
        if (rows == 0) continue;
        int cols = layout[0].size();
        if (cols == 0) continue;

        int leftPad = 0;
        while (leftPad < cols) {
            bool empty = true;
            for (int y = 0; y < rows && empty; ++y)
                if (layout[y][leftPad]) empty = false;
            if (!empty) break;
            ++leftPad;
        }

        int rightPad = cols - 1;
        while (rightPad >= 0) {
            bool empty = true;
            for (int y = 0; y < rows && empty; ++y)
                if (layout[y][rightPad]) empty = false;
            if (!empty) break;
            --rightPad;
        }

        for (int sx = -leftPad; sx < TetrisBoard::WIDTH - rightPad; ++sx) {
            int landY, linesCleared;
            int score = evaluatePlacement(grid, type, rot, sx, landY, linesCleared);
            if (score > best.score && landY >= -rows + 1) {
                best.rotation = rot;
                best.x = sx;
                best.score = score;
            }
        }
    }

    return best;
}

int AIPlayer::evaluatePlacement(const QVector<QVector<TetrisBoard::PieceType>> &grid,
                                 TetrisBoard::PieceType type, int rot, int sx,
                                 int &landY, int &linesCleared)
{
    const auto &layout = m_board->pieceLayout(rot, type);
    int rows = layout.size();
    if (rows == 0) return -1e9;
    int cols = layout[0].size();

    landY = -rows;
    while (true) {
        bool canFall = true;
        for (int py = 0; py < rows && canFall; ++py) {
            for (int px = 0; px < cols && canFall; ++px) {
                if (!layout[py][px]) continue;
                int gy = landY + 1 + py;
                int gx = sx + px;
                if (gy >= TetrisBoard::HEIGHT)
                    canFall = false;
                else if (gy >= 0 && grid[gy][gx] != TetrisBoard::None)
                    canFall = false;
            }
        }
        if (!canFall) break;
        ++landY;
    }

    bool anyOnBoard = false;
    for (int py = 0; py < rows; ++py)
        for (int px = 0; px < cols; ++px)
            if (layout[py][px] && landY + py >= 0 && landY + py < TetrisBoard::HEIGHT)
                anyOnBoard = true;
    if (!anyOnBoard) return -1e9;

    auto virtGrid = grid;
    for (int py = 0; py < rows; ++py) {
        for (int px = 0; px < cols; ++px) {
            if (!layout[py][px]) continue;
            int gy = landY + py;
            int gx = sx + px;
            if (gy >= 0 && gy < TetrisBoard::HEIGHT)
                virtGrid[gy][gx] = type;
        }
    }

    linesCleared = 0;
    for (int y = TetrisBoard::HEIGHT - 1; y >= 0; --y) {
        bool full = true;
        for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
            if (virtGrid[y][x] == TetrisBoard::None) {
                full = false;
                break;
            }
        }
        if (full) {
            ++linesCleared;
            virtGrid.erase(virtGrid.begin() + y);
            virtGrid.insert(virtGrid.begin(), QVector<TetrisBoard::PieceType>(TetrisBoard::WIDTH, TetrisBoard::None));
            ++y;
        }
    }

    QVector<int> heights(TetrisBoard::WIDTH, 0);
    for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
        for (int y = 0; y < TetrisBoard::HEIGHT; ++y) {
            if (virtGrid[y][x] != TetrisBoard::None) {
                heights[x] = TetrisBoard::HEIGHT - y;
                break;
            }
        }
    }

    int holes = 0;
    for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
        bool foundBlock = false;
        for (int y = 0; y < TetrisBoard::HEIGHT; ++y) {
            if (virtGrid[y][x] != TetrisBoard::None)
                foundBlock = true;
            else if (foundBlock)
                ++holes;
        }
    }

    int bump = 0;
    int maxH = 0;
    int sumH = 0;
    for (int x = 0; x < TetrisBoard::WIDTH; ++x) {
        sumH += heights[x];
        if (heights[x] > maxH) maxH = heights[x];
        if (x > 0) bump += std::abs(heights[x] - heights[x - 1]);
    }

    return static_cast<int>(
        linesCleared * WEIGHT_LINES +
        holes * WEIGHT_HOLES +
        bump * WEIGHT_BUMPINESS +
        sumH * WEIGHT_AGG_HEIGHT +
        maxH * WEIGHT_MAX_HEIGHT
    );
}
