#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "tetrisboard.h"

class AIPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AIPlayer(TetrisBoard *board, QObject *parent = nullptr);
    void start();
    void stop();
    bool isRunning() const { return m_timer->isActive(); }

private slots:
    void makeMove();

private:
    struct Move {
        int rotation = 0;
        int x = 0;
        double score = -1e9;
    };

    enum class State { Idle, Rotate, Shift };

    Move findBestMove();
    void copyGrid(QVector<QVector<TetrisBoard::PieceType>> &dst);
    int evaluatePlacement(const QVector<QVector<TetrisBoard::PieceType>> &grid,
                          TetrisBoard::PieceType type, int rot, int sx,
                          int &landY, int &linesCleared);

    TetrisBoard *m_board;
    QTimer *m_timer;
    TetrisBoard::PieceType m_lastPiece = TetrisBoard::None;
    int m_targetRot = 0;
    int m_targetX = 0;
    State m_state = State::Idle;

    static constexpr double WEIGHT_LINES = 100.0;
    static constexpr double WEIGHT_HOLES = -30.0;
    static constexpr double WEIGHT_BUMPINESS = -5.0;
    static constexpr double WEIGHT_AGG_HEIGHT = -3.0;
    static constexpr double WEIGHT_MAX_HEIGHT = -8.0;
};

#endif // AIPLAYER_H
