#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include <QObject>
#include <QPoint>
#include <QVector>
class TetrisBoard : public QObject
{
    Q_OBJECT

public:
    static constexpr int WIDTH = 10;
    static constexpr int HEIGHT = 20;
    static constexpr int PIECE_COUNT = 16;

    enum PieceType { None, I, O, T, S, Z, J, L, F, I2, I3, Plus, L3, U, I5, V, Garbage };

    explicit TetrisBoard(QObject *parent = nullptr);

    void newGame();
    bool isGameOver() const { return m_gameOver; }
    bool isPaused() const { return m_paused; }
    void setPaused(bool p);
    void setPieceSequence(const QVector<PieceType> *seq);

    PieceType cellAt(int x, int y) const;
    PieceType curPieceType() const { return m_curPiece; }
    PieceType nextPieceType() const { return m_nextPiece; }
    QPoint curPos() const { return m_curPos; }
    int curRotation() const { return m_curRotation; }
    int score() const { return m_score; }
    int level() const { return m_level; }
    int lines() const { return m_lines; }

    const QVector<QVector<bool>> &pieceLayout(int rot, PieceType type) const {
        return m_pieces[type].layouts[rot];
    }
    int pieceColorIdx(PieceType type) const { return m_pieces[type].colorIdx; }

    bool moveDown();
    void moveLeft();
    void moveRight();
    void rotate();
    void dropDown();
    void tick();
    void addGarbageLines(int count);

signals:
    void boardChanged();
    void gameOver();
    void scoreChanged(int score, int level, int lines);
    void linesCleared(int count);

private:
    struct Piece {
        QVector<QVector<bool>> layouts[4];
        int colorIdx;
    };

    void initPieces();
    void spawnPiece();
    void lockPiece();
    void clearLines();
    bool collides(int dx, int dy, int rot) const;
    const QVector<QVector<bool>> &currentLayout() const;

    Piece m_pieces[17];
    QVector<QVector<PieceType>> m_grid;

    PieceType m_curPiece = None;
    PieceType m_nextPiece = None;
    QPoint m_curPos;
    int m_curRotation = 0;

    int m_score = 0;
    int m_level = 1;
    int m_lines = 0;

    bool m_gameOver = true;
    bool m_paused = false;

    const QVector<PieceType> *m_pieceSeq = nullptr;
    int m_pieceIdx = 0;
};

#endif // TETRISBOARD_H
