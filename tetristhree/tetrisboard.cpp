#include "tetrisboard.h"
#include <QRandomGenerator>
#include <algorithm>

TetrisBoard::TetrisBoard(QObject *parent)
    : QObject(parent)
{
    m_grid.resize(HEIGHT);
    for (int y = 0; y < HEIGHT; ++y) {
        m_grid[y].resize(WIDTH);
    }
    initPieces();
}

void TetrisBoard::initPieces()
{
    m_pieces[I] = {{
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    }, 0};

    m_pieces[O] = {{
        {{1,1},{1,1}},
        {{1,1},{1,1}},
        {{1,1},{1,1}},
        {{1,1},{1,1}}
    }, 1};

    m_pieces[T] = {{
        {{0,1,0},{1,1,1},{0,0,0}},
        {{0,1,0},{0,1,1},{0,1,0}},
        {{0,0,0},{1,1,1},{0,1,0}},
        {{0,1,0},{1,1,0},{0,1,0}}
    }, 2};

    m_pieces[S] = {{
        {{0,1,1},{1,1,0},{0,0,0}},
        {{0,1,0},{0,1,1},{0,0,1}},
        {{0,0,0},{0,1,1},{1,1,0}},
        {{1,0,0},{1,1,0},{0,1,0}}
    }, 3};

    m_pieces[Z] = {{
        {{1,1,0},{0,1,1},{0,0,0}},
        {{0,0,1},{0,1,1},{0,1,0}},
        {{0,0,0},{1,1,0},{0,1,1}},
        {{0,1,0},{1,1,0},{1,0,0}}
    }, 4};

    m_pieces[J] = {{
        {{1,0,0},{1,1,1},{0,0,0}},
        {{0,1,1},{0,1,0},{0,1,0}},
        {{0,0,0},{1,1,1},{0,0,1}},
        {{0,1,0},{0,1,0},{1,1,0}}
    }, 5};

    m_pieces[L] = {{
        {{0,0,1},{1,1,1},{0,0,0}},
        {{0,1,0},{0,1,0},{0,1,1}},
        {{0,0,0},{1,1,1},{1,0,0}},
        {{1,1,0},{0,1,0},{0,1,0}}
    }, 6};

    m_pieces[F] = {{
        {{0,1,1},{1,1,0},{0,1,0}},
        {{0,1,0},{1,1,1},{0,0,1}},
        {{0,1,0},{0,1,1},{1,1,0}},
        {{1,0,0},{1,1,1},{0,1,0}}
    }, 7};

    m_pieces[I2] = {{
        {{1,1}},
        {{1},{1}},
        {{1,1}},
        {{1},{1}}
    }, 8};

    m_pieces[I3] = {{
        {{1,1,1}},
        {{1},{1},{1}},
        {{1,1,1}},
        {{1},{1},{1}}
    }, 9};

    m_pieces[Plus] = {{
        {{0,1,0},{1,1,1},{0,1,0}},
        {{0,1,0},{1,1,1},{0,1,0}},
        {{0,1,0},{1,1,1},{0,1,0}},
        {{0,1,0},{1,1,1},{0,1,0}}
    }, 10};

    m_pieces[L3] = {{
        {{1,0},{1,1}},
        {{1,1},{1,0}},
        {{1,1},{0,1}},
        {{0,1},{1,1}}
    }, 11};

    m_pieces[U] = {{
        {{1,0,1},{1,1,1}},
        {{1,1},{0,1},{1,1}},
        {{1,1,1},{1,0,1}},
        {{1,1},{1,0},{1,1}}
    }, 12};

    m_pieces[I5] = {{
        {{1,1,1,1,1}},
        {{1},{1},{1},{1},{1}},
        {{1,1,1,1,1}},
        {{1},{1},{1},{1},{1}}
    }, 13};

    m_pieces[V] = {{
        {{1,0,0},{1,0,0},{1,1,1}},
        {{1,1,1},{1,0,0},{1,0,0}},
        {{1,1,1},{0,0,1},{0,0,1}},
        {{0,0,1},{0,0,1},{1,1,1}}
    }, 14};

    m_pieces[Garbage] = {{
        {{1}},
        {{1}},
        {{1}},
        {{1}}
    }, 15};
}

void TetrisBoard::newGame()
{
    for (int y = 0; y < HEIGHT; ++y) {
        m_grid[y].fill(None);
    }
    m_score = 0;
    m_level = 1;
    m_lines = 0;
    m_gameOver = false;
    m_paused = false;

    if (m_pieceSeq) {
        m_pieceIdx = 0;
        m_nextPiece = (*m_pieceSeq)[m_pieceIdx++];
    } else {
        m_nextPiece = static_cast<PieceType>(QRandomGenerator::global()->bounded(1, PIECE_COUNT));
    }
    spawnPiece();
    emit boardChanged();
    emit scoreChanged(m_score, m_level, m_lines);
}

void TetrisBoard::spawnPiece()
{
    m_curPiece = m_nextPiece;
    if (m_pieceSeq && m_pieceIdx < m_pieceSeq->size()) {
        m_nextPiece = (*m_pieceSeq)[m_pieceIdx++];
    } else {
        m_nextPiece = static_cast<PieceType>(QRandomGenerator::global()->bounded(1, PIECE_COUNT));
    }
    m_curRotation = 0;
    m_curPos = QPoint(WIDTH / 2 - 2, -1);

    if (collides(0, 0, 0)) {
        m_gameOver = true;
        emit gameOver();
    }
}

const QVector<QVector<bool>> &TetrisBoard::currentLayout() const
{
    return m_pieces[m_curPiece].layouts[m_curRotation];
}

bool TetrisBoard::collides(int dx, int dy, int rot) const
{
    const auto &layout = m_pieces[m_curPiece].layouts[rot];
    int size = layout.size();
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!layout[y][x]) continue;
            int nx = m_curPos.x() + x + dx;
            int ny = m_curPos.y() + y + dy;
            if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return true;
            if (ny < 0) continue;
            if (m_grid[ny][nx] != None) return true;
        }
    }
    return false;
}

bool TetrisBoard::moveDown()
{
    if (m_gameOver || m_paused) return false;
    if (!collides(0, 1, m_curRotation)) {
        m_curPos.setY(m_curPos.y() + 1);
        emit boardChanged();
        return true;
    }
    lockPiece();
    return false;
}

void TetrisBoard::moveLeft()
{
    if (m_gameOver || m_paused) return;
    if (!collides(-1, 0, m_curRotation)) {
        m_curPos.setX(m_curPos.x() - 1);
        emit boardChanged();
    }
}

void TetrisBoard::moveRight()
{
    if (m_gameOver || m_paused) return;
    if (!collides(1, 0, m_curRotation)) {
        m_curPos.setX(m_curPos.x() + 1);
        emit boardChanged();
    }
}

void TetrisBoard::rotate()
{
    if (m_gameOver || m_paused) return;
    int newRot = (m_curRotation + 1) % 4;
    if (!collides(0, 0, newRot)) {
        m_curRotation = newRot;
        emit boardChanged();
    } else if (!collides(-1, 0, newRot)) {
        m_curRotation = newRot;
        m_curPos.setX(m_curPos.x() - 1);
        emit boardChanged();
    } else if (!collides(1, 0, newRot)) {
        m_curRotation = newRot;
        m_curPos.setX(m_curPos.x() + 1);
        emit boardChanged();
    } else if (!collides(0, -1, newRot)) {
        m_curRotation = newRot;
        m_curPos.setY(m_curPos.y() - 1);
        emit boardChanged();
    }
}

void TetrisBoard::dropDown()
{
    if (m_gameOver || m_paused) return;
    int dropped = 0;
    while (!collides(0, 1, m_curRotation)) {
        m_curPos.setY(m_curPos.y() + 1);
        ++dropped;
    }
    m_score += dropped * 2;
    lockPiece();
}

void TetrisBoard::tick()
{
    if (m_gameOver || m_paused) return;
    moveDown();
}

void TetrisBoard::lockPiece()
{
    const auto &layout = currentLayout();
    int size = layout.size();
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!layout[y][x]) continue;
            int gx = m_curPos.x() + x;
            int gy = m_curPos.y() + y;
            if (gy < 0) {
                m_gameOver = true;
                emit gameOver();
                emit boardChanged();
                return;
            }
            m_grid[gy][gx] = m_curPiece;
        }
    }

    clearLines();
    spawnPiece();
    emit boardChanged();
}

void TetrisBoard::clearLines()
{
    int cleared = 0;
    for (int y = HEIGHT - 1; y >= 0; --y) {
        if (std::all_of(m_grid[y].begin(), m_grid[y].end(),
                        [](PieceType c) { return c != None; })) {
            m_grid.erase(m_grid.begin() + y);
            m_grid.insert(m_grid.begin(), QVector<PieceType>(WIDTH, None));
            ++cleared;
            ++y;
        }
    }

    if (cleared > 0) {
        const int points[] = {0, 100, 300, 500, 800};
        m_lines += cleared;
        m_score += points[cleared] * m_level;
        m_level = m_lines / 10 + 1;
        emit scoreChanged(m_score, m_level, m_lines);
        emit linesCleared(cleared);
    }
}

TetrisBoard::PieceType TetrisBoard::cellAt(int x, int y) const
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return None;

    const auto &layout = currentLayout();
    int size = layout.size();
    int px = x - m_curPos.x();
    int py = y - m_curPos.y();
    if (px >= 0 && px < size && py >= 0 && py < size && layout[py][px]) {
        return m_curPiece;
    }

    return m_grid[y][x];
}

void TetrisBoard::setPaused(bool p)
{
    m_paused = p;
    emit boardChanged();
}

void TetrisBoard::setPieceSequence(const QVector<PieceType> *seq)
{
    m_pieceSeq = seq;
    m_pieceIdx = 0;
}

void TetrisBoard::addGarbageLines(int count)
{
    if (count <= 0) return;
    for (int i = 0; i < count; ++i) {
        m_grid.erase(m_grid.begin());
        QVector<PieceType> row(WIDTH, Garbage);
        int hole = QRandomGenerator::global()->bounded(WIDTH);
        row[hole] = None;
        m_grid.append(row);
    }
    if (m_curPiece != None && collides(0, 0, m_curRotation)) {
        m_gameOver = true;
        emit gameOver();
    }
    emit boardChanged();
}
