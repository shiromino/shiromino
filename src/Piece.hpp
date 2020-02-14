#pragma once

namespace Shiro {
    enum class PieceRotation {
        down, right, up, left
    };

    enum class PieceType {
        I
        J
        L
        X
        S
        Z
        N
        G
        U
        T
        Fa
        Fb
        P
        Q
        W
        Ya
        Yb
        V
        I4
        T4
        J4
        L4
        O
        S4
        Z4
    };

    class Piece {
        PieceType type;
        PieceRotation rotation;
    };
}